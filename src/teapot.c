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
#include "render.h"

static void calcfps(void);
int init_allegro(Camera *cam);

ALLEGRO_DISPLAY *dpy;

Vec3 light_pos = {4, 0, 5};
GLfloat light_ambient[3] = {0.25, 0.20725, 0.20725};
GLfloat light_diffuse[3] = {1, 0.829, 0.829};
GLfloat light_specular[3] = {0.296648, 0.296648, 0.296648};
GLfloat shininess = 0.088*128;

double t = 0.0;

int init_allegro(Camera *cam)
{
	if (!al_init())
		return 1;
	if (!al_install_mouse())
		return 1;
	if (!al_install_keyboard())
		return 1;
	
	al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE |
			ALLEGRO_OPENGL | ALLEGRO_OPENGL_3_0 );	
	al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);

	dpy = al_create_display(cam->left + cam->width, cam->bottom + cam->height);
	glViewport(cam->left, cam->bottom, cam->width, cam->height);

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

int main(int argc, char **argv)
{
	Light light;
	Entity ent;
	Shader *shader;
	const char *filename;
	Camera cam;
	Vec3 position = {0, 0, 5};
	Vec3 up =  {0, 1, 0};
	Vec3 target = {0, 0, 0};
	Quaternion q0 = quat_normalize((Quaternion) {1, 0, 0, 0});
	Quaternion q1 = quat_normalize((Quaternion) {0, 0, -1, 0});
	Quaternion q;
	ALLEGRO_EVENT_QUEUE *ev_queue = NULL;
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
	cam.left = 0;
	cam.bottom = 0;
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

	shader = shader_create(STRINGIFY(ROOT_PATH) "/data/lighting.v.glsl", 
	                       STRINGIFY(ROOT_PATH) "/data/lighting.f.glsl");
	if (shader == NULL)
		return 1;
	glUseProgram(shader->program);

	glmProjectionMatrix = glmNewMatrixStack();
	glmViewMatrix = glmNewMatrixStack();
	glmModelMatrix = glmNewMatrixStack();

	glClearColor(100/255., 149/255., 237/255., 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	light.position = light_pos;
	memcpy(light.ambient, light_ambient, sizeof(light_ambient));
	memcpy(light.diffuse, light_diffuse, sizeof(light_diffuse));
	memcpy(light.specular, light_specular, sizeof(light_specular));
	light.shininess = shininess;

	light_upload_to_gpu(shader, &light);

	ent.position = target;
	ent.orientation = q0;
	ent.type = ENTITY_MESH;
	ent.mesh = mesh;
	entity_upload_to_gpu(shader, &ent);

	/* Transformation matrices */

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
			case ALLEGRO_EVENT_DISPLAY_RESIZE:
				cam.width = ev.display.width;
				cam.height = ev.display.height;
				glmLoadIdentity(glmProjectionMatrix);
				cam_projection_matrix(&cam, glmProjectionMatrix);
				glViewport(cam.left, cam.bottom, cam.width, cam.height);
				break;
			default:
				break;
			}
		}

		/* Start render */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Projection matrix */
		glmLoadIdentity(glmProjectionMatrix);
		cam_projection_matrix(&cam, glmProjectionMatrix);

		/* View matrix */
		glmLoadIdentity(glmViewMatrix);
		cam_view_matrix(&cam, glmViewMatrix);

		/* First the lights */
		/* No model matrix yet, location is directly in world space */
		light.position.x = 5 * cos(M_TWO_PI*t);
		light.position.y = 1;
		light.position.z = 5 * sin(M_TWO_PI*t);
		light_upload_to_gpu(shader, &light);
		
		/* Now the mesh */
		glmPushMatrix(&glmModelMatrix);
			/* The model matrix is set in the entity_render code */
			ent.orientation = q;
			entity_render(shader, &ent);
		glmPopMatrix(&glmModelMatrix);

		al_flip_display();
		calcfps();
		t += 1.0/60/2;
		q = quat_slerp(q0, q1, fabs(fmod(t+1, 2) - 1));
	}

out:
	free(mesh->name);
	free(mesh->vertex);
	free(mesh->normal);
	free(mesh->index);
	free(mesh);

	shader_delete(shader);
	glmFreeMatrixStack(glmProjectionMatrix);
	glmFreeMatrixStack(glmViewMatrix);
	glmFreeMatrixStack(glmModelMatrix);
	al_destroy_display(dpy);
	return 0;
}

