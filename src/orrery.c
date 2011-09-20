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
#include "solarsystem.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846L
#endif

static void calcfps(void);
int init_allegro(Camera *cam);

ALLEGRO_DISPLAY *dpy;
ALLEGRO_EVENT_QUEUE *ev_queue = NULL;
Shader *shader;

Matrix *projectionMatrix = NULL;
Matrix *modelViewMatrix = NULL;

GLfloat light_dir[3] = {4, 0, 5};
GLfloat material_ambient[3] = {0.25, 0.20725, 0.20725};
GLfloat material_diffuse[3] = {1, 0.829, 0.829};
GLfloat material_specular[3] = {0.296648, 0.296648, 0.296648};
GLfloat shininess = 0.088*128;

SolarSystem *solsys;

double t = 0.0;

int init_allegro(Camera *cam)
{
	if (!al_init())
		return 1;
	if (!al_install_mouse())
		return 1;
	if (!al_install_keyboard())
		return 1;
	
	al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_OPENGL | ALLEGRO_OPENGL_FORWARD_COMPATIBLE);
	
	al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);

	dpy = al_create_display(cam->left + cam->width, cam->bottom + cam->height);
	glViewport(cam->left, cam->bottom, cam->width, cam->height);

	ev_queue = al_create_event_queue();
	al_register_event_source(ev_queue, al_get_display_event_source(dpy));
	al_register_event_source(ev_queue, al_get_mouse_event_source());
	al_register_event_source(ev_queue, al_get_keyboard_event_source());

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
		snprintf(string, 64, "%d FPS", (int) (frames/(tick - tock) + 0.5));
		al_set_window_title(dpy, string);

		frames = 0;
		tock = tick;
	}

}

static void upload_uniforms(void)
{

	glmUniformMatrix(glGetUniformLocation(shader->program, "projection_matrix"), projectionMatrix);

	glmUniformMatrix(glGetUniformLocation(shader->program, "modelview_matrix"), modelViewMatrix);

	/* Lighting */
	glUniform3fv(glGetUniformLocation(shader->program, "light_dir"), 1, light_dir);

	glUniform3fv(glGetUniformLocation(shader->program, "mat_ambient"), 1, material_ambient);
	glUniform3fv(glGetUniformLocation(shader->program, "mat_diffuse"), 1, material_diffuse);
	glUniform3fv(glGetUniformLocation(shader->program, "mat_specular"), 1, material_specular);
	glUniform1f(glGetUniformLocation(shader->program, "shininess"), shininess);


}

static void render(Camera *cam, Mesh *mesh)
{
	int i;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glmLoadIdentity(modelViewMatrix);
	cam_view_matrix(cam, modelViewMatrix); /* view */

	for (i = 0; i < solsys->num_planets; i++)
	{
		Vec3 v = kepler_position_at_time(&solsys->planet[i].orbit, t);

		glmPushMatrix(&modelViewMatrix);
		glmTranslate(modelViewMatrix, v.x, v.y, v.z);
		glmScaleUniform(modelViewMatrix, 70000e6);
		upload_uniforms();
		glmPopMatrix(&modelViewMatrix);

		glDrawRangeElements(GL_TRIANGLES, 0, mesh->num_vertices - 1, 
			mesh->num_indices, GL_UNSIGNED_INT, NULL);
	}

	al_flip_display();
	calcfps();
}

int main(int argc, char **argv)
{
	const char *filename;
	Camera cam;
	Vec3 position = {0, 0, 150e9};
	Vec3 up =  {0, 1, 0};
	Vec3 target = {0, 0, 0};
	bool wireframe = false;
	Mesh *mesh;
	if (argc < 2)
		filename = STRINGIFY(ROOT_PATH) "/data/teapot.ply";
	else
		filename = argv[1];

	solsys = solsys_load(STRINGIFY(ROOT_PATH) "/data/sol.ini");
	if (solsys == NULL)
		return 1;

	mesh = mesh_import(filename);
	if (mesh == NULL)
		return 1;

	cam.fov = M_PI/4;
	cam.left = 0;
	cam.bottom = 0;
	cam.width = 1024;
	cam.height = 768;
	cam.zNear = 1e6;
	cam.zFar = 4.5e12;
	init_allegro(&cam);
	cam_lookat(&cam, position, target, up);

	glewInit();

	shader = shader_create(STRINGIFY(ROOT_PATH) "/src/teapot.v.glsl", 
	                       STRINGIFY(ROOT_PATH) "/src/teapot.f.glsl");
	if (shader == NULL)
		return 1;
	glUseProgram(shader->program);

	projectionMatrix = glmNewMatrixStack();
	modelViewMatrix = glmNewMatrixStack();

	glClearColor(100/255., 149/255., 237/255., 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	mesh_upload_to_gpu(mesh, shader->program);

	/* Transformation matrices */
	cam_projection_matrix(&cam, projectionMatrix);

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
				if (ev.mouse.dz != 0)
					cam_dolly(&cam, ev.mouse.dz);
				al_get_mouse_state(&state);
				if (state.buttons & 1)
					cam_orbit(&cam, ev.mouse.dx, -ev.mouse.dy);
				else if (state.buttons & 2)
					cam_rotate(&cam, ev.mouse.dx, -ev.mouse.dy);
				/* The y coordinate needs to be inverted because OpenGL has
				 * the origin in the lower-left and Allegro the upper-left */
				break;
			case ALLEGRO_EVENT_KEY_CHAR:
				if (ev.keyboard.unichar == 'w')
				{
					wireframe = !wireframe;
					if (wireframe)
					{
						glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						glDisable(GL_CULL_FACE);
					}
					else
					{
						glPolygonMode(GL_FRONT, GL_FILL);
						glEnable(GL_CULL_FACE);
					}
				} else if (ev.keyboard.unichar == 'c')
				{
					Vec3 upv = quat_transform(cam.orientation, (Vec3){0, 1, 0});
					cam_lookat(&cam, cam.position, cam.target, upv);
				}
				break;
			default:
				break;
			}
		}

		t += 86400;

		render(&cam, mesh);
	}

out:
	free(mesh->name);
	free(mesh->vertex);
	free(mesh->normal);
	free(mesh->index);
	free(mesh);

	shader_delete(shader);
	glmFreeMatrixStack(projectionMatrix);
	glmFreeMatrixStack(modelViewMatrix);
	al_destroy_display(dpy);
	return 0;
}
