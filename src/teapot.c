#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <allegro5/allegro.h>
#include <ralloc.h>

#include "mathlib.h"
#include "shader.h"
#include "glm.h"
#include "camera.h"
#include "mesh.h"
#include "render.h"
#include "input.h"
#include "util.h"
#include "font.h"
#include "stats.h"

static void calcfps(void);
int init_allegro(Camera *cam);

ALLEGRO_DISPLAY *dpy;

Vec3 light_pos = {4, 0, 5};
GLfloat light_ambient[3] = {0.25, 0.20725, 0.20725};
GLfloat light_diffuse[3] = {1, 0.829, 0.829};
GLfloat light_specular[3] = {0.296648, 0.296648, 0.296648};

double t = 0.0;

static char *fps_string = NULL;

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

	frames++;

	tick = al_get_time();
	if (tick - tock > SAMPLE_TIME)
	{
		if (fps_string)
			ralloc_free(fps_string);

		fps_string = ralloc_asprintf(NULL, "%d FPS", (int) (frames/(tick - tock) + 0.5));
		al_set_window_title(dpy, fps_string);

		frames = 0;
		tock = tick;
	}
}

int main(int argc, char **argv)
{
	Font *font;
	Light light;
	Entity ent1, ent2;
	Shader *shader, *shader_text, *shader_2d;
	Renderable teapot;
	const char *filename;
	Camera cam;
	Vec3 position = {0, 0, 5};
	Vec3 up =  {0, 1, 0};
	Vec3 target = {0, 0, 0};
	Quaternion q0 = quat_normalize((Quaternion) {1, 0, 0, 0});
	Quaternion q1 = quat_normalize((Quaternion) {0, 0, -1, 0});
	Quaternion q;
	ALLEGRO_EVENT_QUEUE *ev_queue = NULL;
	Mesh *mesh;
	if (argc < 2)
		filename = STRINGIFY(ROOT_PATH) "/data/teapot.ply";
	else
		filename = argv[1];

	font = font_load(STRINGIFY(ROOT_PATH) "/data/DejaVuLGCSans.ttf");
	if (font == NULL)
		return 1;

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

	shader_text = shader_create(STRINGIFY(ROOT_PATH) "/data/2D_luminance.v.glsl",
	                            STRINGIFY(ROOT_PATH) "/data/2D_luminance.f.glsl");
	if (shader_text == NULL)
		return 1;

	shader_2d = shader_create(STRINGIFY(ROOT_PATH) "/data/2D_notexture.v.glsl",
	                          STRINGIFY(ROOT_PATH) "/data/2D_notexture.f.glsl");
	if (shader_2d == NULL)
		return 1;

	glmProjectionMatrix = glmNewMatrixStack();
	glmViewMatrix = glmNewMatrixStack();
	glmModelMatrix = glmNewMatrixStack();

	glClearColor(100/255., 149/255., 237/255., 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glCullFace(GL_BACK);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	light.position = light_pos;
	memcpy(light.ambient, light_ambient, sizeof(light_ambient));
	memcpy(light.diffuse, light_diffuse, sizeof(light_diffuse));
	memcpy(light.specular, light_specular, sizeof(light_specular));

	light_upload_to_gpu(&light, shader);

	teapot.data = mesh;
	teapot.upload_to_gpu = mesh_upload_to_gpu;
	teapot.render = mesh_render;
	teapot.shader = shader;
	renderable_upload_to_gpu(&teapot);

	ent1.position = target;
	ent1.orientation = q0;
	ent1.radius = 1;
	ent1.renderable = &teapot;
	ent1.prev = NULL;
	ent1.next = &ent2;

	ent2.position = target;
	ent2.position.y += 1;
	ent2.orientation = (Quaternion) {1/M_SQRT2, 1/M_SQRT2, 0, 0};
	ent2.radius = 1;
	ent2.renderable = &teapot;
	ent2.prev = &ent1;
	ent2.next = NULL;

	stats_begin(font, shader_text, shader_2d);

	/* Start rendering */
	while(handle_input(ev_queue, &cam))
	{

		/* Start render */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Projection matrix */
		glmLoadIdentity(glmProjectionMatrix);
		cam_projection_matrix(&cam, glmProjectionMatrix);

		/* View matrix */
		glmLoadIdentity(glmViewMatrix);
		cam_view_matrix(&cam, glmViewMatrix);

		glUseProgram(shader->program);
		/* First the lights */
		/* No model matrix yet, location is directly in world space */
		light.position.x = 5 * cos(M_TWO_PI*t);
		light.position.y = 1;
		light.position.z = 5 * sin(M_TWO_PI*t);
		light_upload_to_gpu(&light, shader);

		/* Now the mesh */
		/* The model matrix is set in the entity_render code */
		ent2.position.y = ent1.position.y + sin(M_TWO_PI*t/100);
		ent1.orientation = q;
		render_entity_list(&ent1);

		glUseProgram(shader_text->program);
		glmLoadIdentity(glmProjectionMatrix);
		glmOrtho(glmProjectionMatrix, 0, 1024, 0, 768, -1, 1);
		glmUniformMatrix(shader_text->location[SHADER_UNI_P_MATRIX], glmProjectionMatrix);
		stats_render(1024, 768);
		al_flip_display();
		calcfps();
		t += 1.0/60/2;
		q = quat_slerp(q0, q1, fabs(fmod(t+1, 2) - 1));
		stats_end_of_frame();
	}

	ralloc_free(mesh);

	shader_delete(shader);
	glmFreeMatrixStack(glmProjectionMatrix);
	glmFreeMatrixStack(glmViewMatrix);
	glmFreeMatrixStack(glmModelMatrix);
	al_destroy_display(dpy);
	return 0;
}

