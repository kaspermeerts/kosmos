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

#define NUM_SAMPLES 320

static const float AVERAGE_TIME = 1./20;
static Font *font;
static Shader *text_shader, *twod_shader;
static bool inited;
static GLuint graph_vbo, graph_vao;

static struct STATS {
	double start_time; /* When was the stats module inited */
	int frames;
	double tock; /* The time when STATS.frames frames were rendered */
	float fps; /* Current FPS */
	int cur_sample;
	double render_tock;
	float sample[NUM_SAMPLES]; /* Time to render one frame */
} STATS;

void stats_begin(Font *f, Shader *text, Shader *simple)
{
	font = f;
	text_shader = text;
	twod_shader = simple;

	memset(&STATS, '\0', sizeof(STATS));
	STATS.start_time = al_get_time();
	STATS.tock = al_get_time();
	glGenVertexArrays(1, &graph_vao);
	glGenBuffers(1, &graph_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, graph_vbo);
	glBufferData(GL_ARRAY_BUFFER, 2*sizeof(Vertex2) * NUM_SAMPLES, NULL,
			GL_STREAM_DRAW);

	glBindVertexArray(graph_vao);
	glEnableVertexAttribArray(twod_shader->location[SHADER_ATT_POSITION]);
	glVertexAttribPointer(twod_shader->location[SHADER_ATT_POSITION], 2,
			GL_FLOAT, GL_FALSE, sizeof(Vertex2C),
			(void *) offsetof(Vertex2C, x));
	glEnableVertexAttribArray(twod_shader->location[SHADER_ATT_COLOUR]);
	glVertexAttribPointer(twod_shader->location[SHADER_ATT_COLOUR], 4,
			GL_FLOAT, GL_FALSE, sizeof(Vertex2C),
			(void *) offsetof(Vertex2C, r));
	glBindVertexArray(0);
	inited = true;
}

void stats_end(void)
{
	if (!inited)
		return;

	glDeleteBuffers(1, &graph_vbo);
	glDeleteVertexArrays(1, &graph_vao);
}

void stats_end_of_frame(void)
{
	double tick;

	tick = al_get_time();

	STATS.frames++;
	STATS.sample[STATS.cur_sample] = tick - STATS.render_tock;
	STATS.render_tock = al_get_time();
	STATS.cur_sample = (STATS.cur_sample + 1) % NUM_SAMPLES;

	if (tick - STATS.tock > AVERAGE_TIME)
	{
		STATS.fps = STATS.frames / (tick - STATS.tock);
		STATS.frames = 0;
		STATS.tock = tick;
	}
}

static void graph_render(void)
{
	Vertex2C graph[2*NUM_SAMPLES];
	int i;

	glmLoadIdentity(glmModelMatrix);
	glmTranslate(glmModelMatrix, 20, 60, 0);
	glmUniformMatrix(twod_shader->location[SHADER_UNI_M_MATRIX], glmModelMatrix);

	for (i = 0; i < NUM_SAMPLES; i++)
	{
		float sample = STATS.sample[i];
		Vertex2C *bottom = &graph[2*i], *top = &graph[2*i + 1] ;

		bottom->r = 0;
		bottom->g = 1.0;
		bottom->b = 0;
		bottom->a = 1;
		bottom->x = i;
		bottom->y = 0;

		/* Go from green (infinite FPS) to red (<20 FPS) */
		top->r = sample/(1./20.);
		top->g = MAX(1.0 - top->r, 0); /* Don't go under 0 */
		top->b = 0;
		top->a = 1;
		top->x = i;
		top->y = sample*100.0*60;
	}
	glBindVertexArray(graph_vao);
	glBindBuffer(GL_ARRAY_BUFFER, graph_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(graph), graph, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDrawArrays(GL_LINES, 0, 2*NUM_SAMPLES);

	glBindVertexArray(0);

}

void stats_render(int width, int height)
{
	if (!inited)
	{
		log_err("Stats used before init\n");
		return;
	}

	/* TODO renderer_set_2D or something */
	glUseProgram(text_shader->program);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glmLoadIdentity(glmProjectionMatrix);
	glmOrtho(glmProjectionMatrix, 0, width, 0, height, -1, 1);
	glmUniformMatrix(text_shader->location[SHADER_UNI_P_MATRIX],
			glmProjectionMatrix);

	glmLoadIdentity(glmViewMatrix);

	glmLoadIdentity(glmModelMatrix);
	glmTranslate(glmModelMatrix, 20, 20, 0);
	text_create_and_render(text_shader, font, 16, "FPS: %d",
			(int) (STATS.fps + 0.5));
	glmTranslate(glmModelMatrix, 0, 20, 0);
	text_create_and_render(text_shader, font, 16, "Time: %f",
			al_get_time() - STATS.start_time);

	glUseProgram(twod_shader->program);
	glmUniformMatrix(twod_shader->location[SHADER_UNI_P_MATRIX],
			glmProjectionMatrix);
	glmUniformMatrix(twod_shader->location[SHADER_UNI_V_MATRIX],
			glmViewMatrix);

	graph_render();

	/* TODO: renderer_set_3D */
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	return;
}
