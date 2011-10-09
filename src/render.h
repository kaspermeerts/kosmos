#ifndef KOSMOS_RENDER
#define KOSMOS_RENDER

#include "mathlib.h"
#include "glm.h"
#include "mesh.h"
#include "shader.h"
#include "camera.h"
#include "solarsystem.h"

typedef void (*upload_cb)(void *self, Shader *shader);
typedef void (*render_cb)(void *self, Shader *shader);

typedef struct Light {
	Vec3 position; /* Position in model space */

	float ambient[3];
	float diffuse[3];
	float specular[3];
} Light;

typedef struct Renderable {
	GLuint vao; /* Vertex Array Object */

	upload_cb upload_to_gpu;
	render_cb render;

	enum {MEMLOC_RAM, MEMLOC_GPU} memloc;

	void *data;
} Renderable;

typedef struct Entity {
	struct Entity *next;
	Vec3 position;
	Quaternion orientation;
	double scale;

	Renderable *renderable;
} Entity;

void render_entity_list(Entity *ent, Shader *shader);
void entity_render(Entity *ent, Shader *shader);
void mesh_upload_to_gpu(void *mesh, Shader *shader);
void light_upload_to_gpu(void *light, Shader *shader);
void renderable_upload_to_gpu(Renderable *obj, Shader *shader);
void mesh_render(void *self, Shader *shader);

#endif
