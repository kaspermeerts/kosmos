#include <GL/glew.h>
#include <GL/gl.h>

#include "mesh.h"

void mesh_upload_to_gpu(Mesh *mesh, GLuint program)
{
	/* TODO: give shader as argument */
	/* TODO: So much error-checking */
	GLint vertex_attr, normal_attr;
	

	/* Vertices */
	glGenBuffers(1, &mesh->vertex_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh->num_vertices * sizeof(Vertex),
			mesh->vertex, GL_STATIC_DRAW);
	vertex_attr = glGetAttribLocation(program, "in_position");
	glEnableVertexAttribArray(vertex_attr);
	glVertexAttribPointer(vertex_attr, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	
	/* Normals */
	glGenBuffers(1, &mesh->normal_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->normal_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh->num_normals * sizeof(Normal),
			mesh->normal, GL_STATIC_DRAW);
	normal_attr = glGetAttribLocation(program, "in_normal");
	glEnableVertexAttribArray(normal_attr);
	glVertexAttribPointer(normal_attr, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

	/* Triangles */
	glGenBuffers(1, &mesh->triangle_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->triangle_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->num_triangles * sizeof(Triangle),
			mesh->triangle, GL_STATIC_DRAW);

}
