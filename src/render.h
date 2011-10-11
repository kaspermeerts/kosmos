#ifndef KOSMOS_RENDER
#define KOSMOS_RENDER

#include "mathlib.h"
#include "glm.h"
#include "mesh.h"
#include "shader.h"
#include "camera.h"
#include "solarsystem.h"


typedef struct Light {
	Vec3 position; /* Position in model space */

	float ambient[3];
	float diffuse[3];
	float specular[3];
} Light;

typedef struct Renderable {
	Shader *shader;
	GLuint vao; /* Vertex Array Object */

	void (*upload_to_gpu)(struct Renderable *o);
	void (*render)(struct Renderable *o);

	enum {MEMLOC_RAM, MEMLOC_GPU} memloc;

	void *data;
} Renderable;

typedef struct Entity {
	struct Entity *prev, *next;
	Vec3 position;
	Quaternion orientation;
	double radius;

	Renderable *renderable;
} Entity;

void render_entity_list(Entity *ent);
void light_upload_to_gpu(void *light, Shader *shader);

void renderable_upload_to_gpu(Renderable *obj);
void mesh_upload_to_gpu(Renderable *obj);
void point_upload_to_gpu(Renderable *obj);
void renderable_render(Renderable *ent);
void mesh_render(Renderable *obj);
void point_render(Renderable *obj);

#endif
