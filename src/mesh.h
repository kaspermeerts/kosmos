#ifndef _MESH_H_
#define _MESH_H_

#include <GL/gl.h>

typedef struct Vertex {
	GLfloat x, y, z;
} Vertex;

typedef Vertex Normal;

typedef struct TexCoord {
	GLfloat u, v;
} TexCoord;

typedef struct Triangle {
	GLuint vertex[3];
	GLuint normal[3];
	GLuint texcoord[3];
} Triangle;

typedef struct Mesh {
	unsigned int num_vertices;
	Vertex *vertex;
	unsigned int num_normals;
	Normal *normal;
	unsigned int num_texcoords;
	TexCoord *texcoord;
	unsigned int num_triangles;
	Triangle *triangle;
	unsigned int num_groups; /* XXX */
} Mesh;

Mesh *load_mesh(const char *filename);
#endif
