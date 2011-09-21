#ifndef KOSMOS_RENDER
#define KOSMOS_RENDER

#include "mathlib.h"
#include "glm.h"
#include "mesh.h"
#include "shader.h"
#include "camera.h"

typedef struct Light {
	float dir[3];

	float ambient[3];
	float diffuse[3];
	float specular[3];

	float shininess; /* FIXME: Is a material dependent property */
} Light;

typedef struct Entity {
	enum { TYPE_MESH, TYPE_PATH, TYPE_LODSPHERE } type;

	union {
		struct {
			Vec3 position;
			Quaternion orientation;
		} common;
		struct {
			Vec3 position;
			Quaternion orientation;
			Mesh *mesh;
		} mesh;
	} data;
} Entity;

void entity_upload_to_gpu(Shader *shader, Entity *ent);
void entity_render(Shader *shader, Entity *ent);
void lights_upload_to_gpu(Shader *shader);

/* TODO: Cleanup globals */
extern Light g_light;
extern Matrix *projection_matrix;
extern Matrix *modelview_matrix;

#endif
