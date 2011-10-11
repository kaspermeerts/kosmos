#include <stdbool.h>
#include <ralloc.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <allegro5/allegro.h>

#include "shader.h"
#include "glm.h"
#include "stats.h"
#include "font.h"
#include "log.h"
#include "mesh.h" /* XXX: For Vertex */

#define NUM_SAMPLES 640

static const float SAMPLE_TIME = 0.250;
static Font *font;
static Shader *text_shader, *simple_shader;
static bool inited;
static GLuint graph_vbo;

static struct STATS {
	double start_time;
	int frames;
	double tock; /* The time when STATS.frames frames were rendered */
	float fps;
	int cur_sample;
	float sample[NUM_SAMPLES]; /* Sample of how long the rendering took */
} STATS;

void stats_begin(Font *f, Shader *text, Shader *simple)
{
	font = f;
	text_shader = text;
	simple_shader = simple;

	memset(&STATS, '\0', sizeof(STATS));
	STATS.start_time = al_get_time();
	STATS.tock = al_get_time();
	glGenBuffers(1, &graph_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, graph_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * NUM_SAMPLES, NULL,
			GL_DYNAMIC_DRAW);

	inited = true;
}

void stats_end_of_frame(void)
{
	double tick;

	STATS.frames++;
	tick = al_get_time();

	if (tick - STATS.tock > SAMPLE_TIME)
	{
		STATS.sample[STATS.cur_sample] = (tick - STATS.tock) / STATS.frames;
		STATS.cur_sample = (STATS.cur_sample + 1) % NUM_SAMPLES;
		STATS.fps = STATS.frames / (tick - STATS.tock);
		STATS.frames = 0;
		STATS.tock = tick;
	}
}

void stats_render(int width, int height)
{
	Matrix projection;
	Vertex graph[2 + NUM_SAMPLES];
	int i;

	if (!inited)
	{
		log_err("Stats used before init\n");
		return;
	}

	/* TODO renderer_set_2D or something */
	glUseProgram(text_shader->program);
	glmLoadIdentity(&projection);
	glmOrtho(&projection, 0, width, 0, height, -1, 1);
	glmUniformMatrix(text_shader->location[SHADER_UNI_P_MATRIX], &projection);
	text_create_and_render(text_shader, font, 20, 20, "FPS: %d",
			(int) (STATS.fps + 0.5));
	text_create_and_render(text_shader, font, 20, 40, "Time: %f",
			al_get_time() - STATS.start_time);

	glUseProgram(simple_shader->program);
	glmUniformMatrix(simple_shader->location[SHADER_UNI_P_MATRIX], &projection);
	glmLoadIdentity(&projection);
	glmUniformMatrix(simple_shader->location[SHADER_UNI_V_MATRIX], &projection);
	glmUniformMatrix(simple_shader->location[SHADER_UNI_M_MATRIX], &projection);
	graph[0].x = 20 + NUM_SAMPLES;
	graph[0].y = 60;
	graph[0].z = 0;
	graph[1].x = 20;
	graph[1].y = 60;
	graph[1].z = 0;
	for (i = 0; i < NUM_SAMPLES; i++)
	{
		graph[i+2].x = 20 + i;
		graph[i+2].y = 60 + 10*STATS.sample[i]*100.0*60;
		graph[i+2].z = 0;
	}
	glBindBuffer(GL_ARRAY_BUFFER, graph_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*(NUM_SAMPLES+2), graph,
			GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(simple_shader->location[SHADER_ATT_POSITION]);
	glVertexAttribPointer(simple_shader->location[SHADER_ATT_POSITION], 3,
			GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
	glDrawArrays(GL_LINE_STRIP, 0, NUM_SAMPLES+2);
	glDisableVertexAttribArray(simple_shader->location[SHADER_ATT_POSITION]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return;
}
