#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <allegro5/allegro.h>

#include "quaternion.h"
#include "glm.h"
#include "teapot.h"
#include "camera.h"
#include "vector.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846L
#endif

static void calcfps(void);

ALLEGRO_DISPLAY *dpy;
GLuint shaderProgram;

Matrix *projectionMatrix = NULL;
Matrix *modelviewMatrix = NULL;

GLfloat light_dir[3] = {4, 0, 5};
GLfloat material_ambient[3] = {0.25, 0.20725, 0.20725};
GLfloat material_diffuse[3] = {1, 0.829, 0.829};
GLfloat material_specular[3] = {0.296648, 0.296648, 0.296648};
GLfloat shininess = 0.088*128;

float theta = 0.0;

const char *vertexSource = 
"#version 330 core\n"
"\n"
"uniform mat4 projection_matrix;"
"uniform mat4 modelview_matrix;"
"uniform vec3 light_dir;"
"uniform vec3 mat_ambient, mat_diffuse, mat_specular;"
"uniform float shininess;"
""
"in vec4 in_position;"
"in vec3 in_normal;"
""
"out vec3 L, E, vertNormal;"
""
"void main(void)"
"{"
"	vec4 eye_pos = modelview_matrix * in_position;"
"	gl_Position = projection_matrix * eye_pos;"
""
"	mat3 normal_matrix = inverse(transpose(mat3(modelview_matrix)));"
"	L = normalize(mat3(modelview_matrix) * light_dir);"
"	E = -normalize(eye_pos.xyz);"
"	vertNormal = normal_matrix * in_normal;"
"}"
;

const char *fragmentSource =
"#version 330 core\n"
"\n"
"uniform mat4 projection_matrix;"
"uniform mat4 modelview_matrix;"
"uniform vec3 light_dir;"
"uniform vec3 mat_ambient, mat_diffuse, mat_specular;"
"uniform float shininess;"
""
"in vec3 L, E, vertNormal;"
""
"out vec4 fragColour;"
""
"void main(void)"
"{"
"	vec3 N, R;"
"	float NdotL, RdotE;"
""
"	N = normalize(vertNormal);"
"	R = reflect(-L, N);"
"	NdotL = max(dot(N, L), 0);"
"	RdotE = max(dot(R, E), 0);"
""
"	fragColour = vec4(mat_ambient + NdotL * mat_diffuse +"
"			pow(RdotE, shininess) * mat_specular, 1);"
""
"}";

int init_allegro(void)
{
	if (!al_init())
		return 1;
	if (!al_install_mouse())
		return 1;
	if (!al_install_keyboard())
		return 1;
	
	al_set_new_display_flags(ALLEGRO_WINDOWED | 
			ALLEGRO_OPENGL_FORWARD_COMPATIBLE);
	
	al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);

	dpy = al_create_display(1024, 768);

	return 0;
}

static void calcfps()
{
	const double SAMPLE_TIME = 0.250;
	static int frames;
	static double tock=0.0;
	double tick;
	char string[64];

	frames++;

	tick = al_get_time();
	if (tick - tock > SAMPLE_TIME)
	{
		tock = tick;
		snprintf(string, 64, "%d FPS", (int) (frames / SAMPLE_TIME));
		al_set_window_title(dpy, string);

		frames = 0;
	}

}

void show_info_log(GLuint object, PFNGLGETSHADERIVPROC glGet__iv,
		PFNGLGETSHADERINFOLOGPROC glGet__InfoLog)
{
	GLint log_len;
	char *log;

	glGet__iv(object, GL_INFO_LOG_LENGTH, &log_len);
	log = malloc(log_len);
	glGet__InfoLog(object, log_len, NULL, log);
	fprintf(stderr, "%s", log);
	free(log);
	return;
}

int init_shaders()
{
	GLuint vertexShader, fragmentShader;
	GLint linked;

	shaderProgram = glCreateProgram();

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, (const GLchar **) &vertexSource, 0);
	glCompileShader(vertexShader);
	show_info_log(vertexShader, glGetShaderiv, glGetShaderInfoLog);
	glAttachShader(shaderProgram, vertexShader);

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, (const GLchar **) &fragmentSource, 0);
	glCompileShader(fragmentShader);
	show_info_log(fragmentShader, glGetShaderiv, glGetShaderInfoLog);
	glAttachShader(shaderProgram, fragmentShader);

	glBindFragDataLocation(shaderProgram, 0, "fragColour");

	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);

	return linked;
}

int main(void)
{
	Camera cam;
	Vec3 position = {0.0, 0.0, 5};
	Vec3 up =  {0.0, 1.0, 0.0};
	Vec3 target = {1.0, 0.0, 0.0};
	ALLEGRO_EVENT_QUEUE *ev_queue = NULL;
	GLuint vbo_vertices, vbo_indices, vbo_normals;
	GLint proj_unif, view_unif;
	GLint pos_attr, nor_attr;
	bool wireframe = false;

	init_allegro();

	ev_queue = al_create_event_queue();
	al_register_event_source(ev_queue, al_get_display_event_source(dpy));
	al_register_event_source(ev_queue, al_get_mouse_event_source());
	al_register_event_source(ev_queue, al_get_keyboard_event_source());

	glewInit();

	init_shaders();
	show_info_log(shaderProgram, glGetProgramiv, glGetProgramInfoLog);

	projectionMatrix = glmNewMatrixStack();
	modelviewMatrix = glmNewMatrixStack();

	glClearColor(100/255., 149/255., 237/255., 1.0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	/* Make buffers */
	glGenBuffers(1, &vbo_vertices);
	glGenBuffers(1, &vbo_indices);
	glGenBuffers(1, &vbo_normals);

	/* Indices: no attrib array necessary */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(teapot_indices),
			teapot_indices, GL_STATIC_DRAW);

	/* Position vertices */
	pos_attr = glGetAttribLocation(shaderProgram, "in_position");
	glEnableVertexAttribArray(pos_attr);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(teapot_vertices), 
			teapot_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(pos_attr, 3, GL_FLOAT, GL_FALSE,
			0, (void *) (-3*4));

	/* Normals */
	nor_attr = glGetAttribLocation(shaderProgram, "in_normal");
	glEnableVertexAttribArray(nor_attr);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(teapot_normals),
			teapot_normals, GL_STATIC_DRAW);
	glVertexAttribPointer(nor_attr, 3, GL_FLOAT, GL_FALSE,
			0, (void *) (-3*4));
		

	/* Transformation matrices */
	cam.fov = M_PI/12;
	cam.aspect = 4./3;
	cam.zNear = 1;
	cam.zFar = 100;
	cam_lookat(&cam, position, target, up);

	cam_projection_matrix(&cam, projectionMatrix);
	proj_unif = glGetUniformLocation(shaderProgram, "projection_matrix");
	glmUniformMatrix(proj_unif, projectionMatrix);

	view_unif = glGetUniformLocation(shaderProgram, "modelview_matrix");

	/* Lighting */
	glUniform3fv(glGetUniformLocation(shaderProgram, "light_dir"), 1, light_dir);

	glUniform3fv(glGetUniformLocation(shaderProgram, "mat_ambient"), 1, material_ambient);
	glUniform3fv(glGetUniformLocation(shaderProgram, "mat_diffuse"), 1, material_diffuse);
	glUniform3fv(glGetUniformLocation(shaderProgram, "mat_specular"), 1, material_specular);
	glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), shininess);

	/* Start rendering */

	while(1)
	{
		ALLEGRO_EVENT ev;
		ALLEGRO_MOUSE_STATE state;

		while (al_get_next_event(ev_queue, &ev))
		{
			switch (ev.type)
			{
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				return 0;
				break;
			case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
				al_show_mouse_cursor(dpy);
				al_ungrab_mouse();
				break;
			case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
				al_hide_mouse_cursor(dpy);
				al_grab_mouse(dpy);
				break;
			case ALLEGRO_EVENT_MOUSE_AXES:
				al_get_mouse_state(&state);
				if (state.buttons & 1)
					cam_orbit(&cam, ev.mouse.dx, -ev.mouse.dy);
				/* The y coordinate needs to be inverted because OpenGL has
				 * the origin in the lower-left and Allegro the upper-left */
				break;
			case ALLEGRO_EVENT_KEY_CHAR:
				if(ev.keyboard.unichar == 'w')
					wireframe = !wireframe;
				break;
			default:
				break;
			}
		}
		
		glmLoadIdentity(modelviewMatrix);
		cam_view_matrix(&cam, modelviewMatrix); /* view */
		glmTranslate(modelviewMatrix, 1, 0, 0);
		glmScaleUniform(modelviewMatrix, 0.015); /* model */
		glmUniformMatrix(view_unif, modelviewMatrix);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (wireframe)
			glDrawRangeElements(GL_LINES, 0, NUM_INDICES - 1, 
				NUM_INDICES, GL_UNSIGNED_SHORT, NULL);
		else
			glDrawRangeElements(GL_TRIANGLES, 0, NUM_INDICES - 1,
				NUM_INDICES, GL_UNSIGNED_SHORT, NULL);

		al_flip_display();
		calcfps();
	}

	glDeleteBuffers(1, &vbo_vertices);
	glDeleteBuffers(1, &vbo_indices);
	glDeleteBuffers(1, &vbo_normals);
	glmFreeMatrixStack(projectionMatrix);
	glmFreeMatrixStack(modelviewMatrix);
	al_destroy_display(dpy);
	return 0;
}

