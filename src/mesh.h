#ifndef _MESH_H_
#define _MESH_H_

#include <rply.h>
#include <stdbool.h>
#include <GL/gl.h>

typedef struct Vertex {
	GLfloat x, y, z;
} __attribute__((packed)) Vertex;

typedef Vertex Normal;

typedef struct TexCoord {
	GLfloat u, v;
} __attribute__((packed)) TexCoord;

typedef struct Triangle {
	GLushort vertex[3];
} __attribute__((packed)) Triangle ;

typedef struct Mesh {
	char *name;

	int num_vertices;
	Vertex *vertex;
	GLuint vertex_vbo;

	int num_normals;
	Normal *normal;
	GLuint normal_vbo;

	int num_texcoords;
	TexCoord *texcoord;
	GLuint texcoord_vbo;

	int num_triangles;
	Triangle *triangle;
	GLuint triangle_vbo;
} Mesh;

Mesh *mesh_import(const char *filename);
bool mesh_load_ply(Mesh *mesh, p_ply ply);

void mesh_upload_to_gpu(Mesh *mesh, GLuint program);
#endif
