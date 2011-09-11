#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <allegro5/allegro.h>

#define STRINGIFY(s) XSTRINGIFY(s)
#define XSTRINGIFY(s) #s

#include "mathlib.h"
#include "shader.h"
#include "glm.h"
#include "camera.h"
#include "mesh.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846L
#endif

static void calcfps(void);

ALLEGRO_DISPLAY *dpy;

Matrix *projectionMatrix = NULL;
Matrix *modelviewMatrix = NULL;

GLfloat light_dir[3] = {4, 0, 5};
GLfloat material_ambient[3] = {0.25, 0.20725, 0.20725};
GLfloat material_diffuse[3] = {1, 0.829, 0.829};
GLfloat material_specular[3] = {0.296648, 0.296648, 0.296648};
GLfloat shininess = 0.088*128;

float theta = 0.0;

int init_allegro(Camera *cam)
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

	dpy = al_create_display(cam->x + cam->width, cam->y + cam->height);
	glViewport(cam->x, cam->y, cam->width, cam->height);

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

int main(int argc, char **argv)
{
	Shader *shader;
	const char *filename;
	Camera cam;
	Vec3 position = {0, 0, 5};
	Vec3 up =  {0, 1, 0};
	Vec3 target = {0, 0, 0};
	ALLEGRO_EVENT_QUEUE *ev_queue = NULL;
	GLint proj_unif, view_unif;
	bool wireframe = false;
	Mesh *mesh;
	if (argc < 2)
		filename = STRINGIFY(ROOT_PATH) "/data/teapot.ply";
	else
		filename = argv[1];

	mesh = mesh_import(filename);
	if (mesh == NULL)
		return 1;

	cam.fov = M_PI/12;
	cam.x = 0;
	cam.y = 0;
	cam.width = 1024;
	cam.height = 768;
	cam.zNear = 1;
	cam.zFar = 100;
	init_allegro(&cam);
	cam_lookat(&cam, position, target, up);

	ev_queue = al_create_event_queue();
	al_register_event_source(ev_queue, al_get_display_event_source(dpy));
	al_register_event_source(ev_queue, al_get_mouse_event_source());
	al_register_event_source(ev_queue, al_get_keyboard_event_source());

	glewInit();

	shader = shader_create(STRINGIFY(ROOT_PATH) "/src/teapot.v.glsl", 
	                       STRINGIFY(ROOT_PATH) "/src/teapot.f.glsl");
	if (shader == NULL)
		return 1;
	glUseProgram(shader->program);

	projectionMatrix = glmNewMatrixStack();
	modelviewMatrix = glmNewMatrixStack();

	glClearColor(100/255., 149/255., 237/255., 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	/* Make buffers */


	mesh_upload_to_gpu(mesh, shader->program);

	/* Transformation matrices */
	cam_projection_matrix(&cam, projectionMatrix);
	proj_unif = glGetUniformLocation(shader->program, "projection_matrix");
	glmUniformMatrix(proj_unif, projectionMatrix);

	view_unif = glGetUniformLocation(shader->program, "modelview_matrix");

	/* Lighting */
	glUniform3fv(glGetUniformLocation(shader->program, "light_dir"), 1, light_dir);

	glUniform3fv(glGetUniformLocation(shader->program, "mat_ambient"), 1, material_ambient);
	glUniform3fv(glGetUniformLocation(shader->program, "mat_diffuse"), 1, material_diffuse);
	glUniform3fv(glGetUniformLocation(shader->program, "mat_specular"), 1, material_specular);
	glUniform1f(glGetUniformLocation(shader->program, "shininess"), shininess);

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
				goto out;
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
				{
					wireframe = !wireframe;
					if (wireframe)
						glDisable(GL_CULL_FACE);
					else
						glEnable(GL_CULL_FACE);
				}
				break;
			default:
				break;
			}
		}


		glmLoadIdentity(modelviewMatrix);
		cam_view_matrix(&cam, modelviewMatrix); /* view */
		glmTranslate(modelviewMatrix, target.x, target.y, target.z);
		glmUniformMatrix(view_unif, modelviewMatrix);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT, GL_FILL);

		glDrawRangeElements(GL_TRIANGLES, 0, mesh->num_vertices - 1, 
			mesh->num_triangles*3, GL_UNSIGNED_INT, NULL);

		al_flip_display();
		calcfps();
	}

out:
	free(mesh->name);
	free(mesh->vertex);
	free(mesh->triangle);
	free(mesh);

	shader_delete(shader);
	glmFreeMatrixStack(projectionMatrix);
	glmFreeMatrixStack(modelviewMatrix);
	al_destroy_display(dpy);
	return 0;
}

