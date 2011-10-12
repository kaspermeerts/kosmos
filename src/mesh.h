#ifndef KOSMOS_MESH_H
#define KOSMOS_MESH_H

#include <GL/gl.h>
#include "glm.h"

typedef struct TexCoord {
	GLfloat u, v;
} TexCoord;

typedef struct Mesh {
	char *name;

	GLuint vbo;
	int num_vertices;
	Vertex3N *vertex;

	GLenum type; /* Triangles or Quads */
	int num_indices;
	GLuint *index;
	GLuint ibo;
} Mesh;

Mesh *mesh_import(const char *filename);
void mesh_unitize(Mesh *mesh);

#endif
