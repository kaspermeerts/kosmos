#ifndef KOSMOS_MESH_H
#define KOSMOS_MESH_H

#include <GL/gl.h>

typedef struct Vertex {
	GLfloat x, y, z;
	GLfloat nx, ny, nz;
} Vertex;

typedef Vertex Normal;

typedef struct TexCoord {
	GLfloat u, v;
} TexCoord;

typedef struct Mesh {
	char *name;

	int num_vertices;
	Vertex *vertex;
	GLuint vertex_vbo;

	int num_normals;
	Normal *normal;
	GLuint normal_vbo;
#if 0 /* TODO: Textures :p */
	int num_texcoords;
	TexCoord *texcoord;
	GLuint texcoord_vbo;
#endif
	GLenum type; /* Triangles or Quads */
	int num_indices;
	GLuint *index;
	GLuint ibo;
} Mesh;

Mesh *mesh_import(const char *filename);

#endif
