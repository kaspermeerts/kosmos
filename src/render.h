#ifndef KOSMOS_RENDER
#define KOSMOS_RENDER

#include "mathlib.h"
#include "glm.h"
#include "mesh.h"
#include "shader.h"
#include "camera.h"

typedef struct Light {
	Vec3 position; /* Position in model space */

	float ambient[3];
	float diffuse[3];
	float specular[3];

	float shininess; /* FIXME: Is a material dependent property */
} Light;

typedef struct Entity {
	enum { ENTITY_MESH, ENTITY_PATH, ENTITY_LODSPHERE } type;

	/* Common stuff */
	Vec3 position; /* XXX: Maybe make this a different type: Point3 */
	Quaternion orientation;

	GLuint vao; /* Vertex Array Object */

	/* TODO: Put all of this in a union */
	Mesh *mesh;
} Entity;

void entity_upload_to_gpu(Shader *shader, Entity *ent);
void entity_render(Shader *shader, Entity *ent);
void light_upload_to_gpu(Shader *shader, Light *light);

/* XXX: I'm planning on keeping these as globals. After all, it's how
 * OpenGL used to do it.*/
extern Matrix *projection_matrix;
extern Matrix *modelview_matrix;

#endif
