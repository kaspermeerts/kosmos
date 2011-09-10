#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <allegro5/allegro.h>

#include "mathlib.h"
#include "mesh.h"

enum vertex_props {
	PROP_X,
	PROP_Y,
	PROP_Z 
};

extern char *strdup(const char *s); /* FIXME */

static int vertex_cb(p_ply_argument argument);
static int face_cb(p_ply_argument argument);
static void generate_normals(Mesh *mesh);
static Normal calc_face_normal(Vertex v1, Vertex v2, Vertex v3);

Mesh *mesh_import(const char *filename)
{
	Mesh *mesh;
	p_ply ply;
	const char *tail;

	ply = ply_open(filename, NULL);
	if (ply == NULL)
		return NULL;
	if (ply_read_header(ply) == 0)
		return NULL;

	mesh = malloc(sizeof(Mesh));

	tail = strrchr(filename, '/');
	mesh->name = strdup((tail ? tail + 1 : filename));
	if (mesh_load_ply(mesh, ply) == false)
	{
		/* TODO: log */
		free(mesh);
		mesh = NULL;
	}

	generate_normals(mesh);

	ply_close(ply);

	return mesh;
}

bool mesh_load_ply(Mesh *mesh, p_ply ply)
{
	p_ply_element element = NULL;

	/* TODO: Decent error handling */
	while((element = ply_get_next_element(ply, element)) != NULL)
	{
		const char *name;
		long ninstances;
		ply_get_element_info(element, &name, &ninstances);

		if (strcmp(name, "vertex") == 0)
		{
			mesh->num_vertices = ninstances;
			mesh->vertex = calloc(ninstances, sizeof(Vertex));
		} else if (strcmp(name, "face") == 0)
		{
			mesh->num_triangles = ninstances;
			mesh->triangle = calloc(ninstances, sizeof(Triangle));
		}
	}

	ply_set_read_cb(ply, "vertex", "x", vertex_cb, mesh, (long) PROP_X);
	ply_set_read_cb(ply, "vertex", "y", vertex_cb, mesh, (long) PROP_Y);
	ply_set_read_cb(ply, "vertex", "z", vertex_cb, mesh, (long) PROP_Z);
	ply_set_read_cb(ply, "face", "vertex_indices", face_cb, mesh, 0);

	ply_read(ply);
	return true;
}

static int vertex_cb(p_ply_argument argument)
{
	Mesh *mesh;
	void *pdata;
	long idata, index;
	double data;

	ply_get_argument_user_data(argument, &pdata, &idata);
	mesh = (Mesh *)pdata;
	ply_get_argument_element(argument, NULL, &index);
	data = ply_get_argument_value(argument);
	switch(idata)
	{
	case PROP_X:
		mesh->vertex[index].x = data;
		break;
	case PROP_Y:
		mesh->vertex[index].y = data;
		break;
	case PROP_Z:
		mesh->vertex[index].z = data;
		break;
	default:
		/* Shouldn't happen */
		fprintf(stderr, "Internal consistency error\n");
		return 0;
		break;
	}

	return 1;
}

static int face_cb(p_ply_argument argument)
{
	Mesh *mesh;
	void *pdata;
	long index, len, value_index;
	
	ply_get_argument_user_data(argument, &pdata, NULL);
	mesh = (Mesh *)pdata;
	ply_get_argument_element(argument, NULL, &index);
	ply_get_argument_property(argument, NULL, &len, &value_index);
	
	if (len != 3)
	{
		/* TODO: Tesselate? */
		fprintf(stderr, "Malformed face\n");
		return 1;
	}
	
	if (value_index == -1)
		return 1;

	mesh->triangle[index].vertex[value_index] = ply_get_argument_value(argument);

	return 1;
}

/* Generate normals for a mesh based on the faces.
 * We assume all faces are triangles here. */
static void generate_normals(Mesh *mesh)
{
	struct {
		int num_faces;
		Normal normal_sum;
	} *normal_record;
	int i;

	/* 1. Initialize all normal sums to zero */
	normal_record = calloc(mesh->num_vertices, sizeof(*normal_record));
	for (i = 0; i < mesh->num_vertices; i++)
	{
		normal_record[i].num_faces = 0;
		normal_record[i].normal_sum.x = 0.0;
		normal_record[i].normal_sum.y = 0.0;
		normal_record[i].normal_sum.z = 0.0;
	}

	/* 2. Accumulate the normals of each face onto the vertices */
	for (i = 0; i < mesh->num_triangles; i++)
	{
		Normal normal;
		GLuint i1, i2, i3;

		i1 = mesh->triangle[i].vertex[0];
		i2 = mesh->triangle[i].vertex[1];
		i3 = mesh->triangle[i].vertex[2];

		normal = calc_face_normal(mesh->vertex[i1], mesh->vertex[i2], 
				mesh->vertex[i3]);

		normal_record[i1].normal_sum.x += normal.x;
		normal_record[i1].normal_sum.y += normal.y;
		normal_record[i1].normal_sum.z += normal.z;
		normal_record[i1].num_faces++;

		normal_record[i2].normal_sum.x += normal.x;
		normal_record[i2].normal_sum.y += normal.y;
		normal_record[i2].normal_sum.z += normal.z;
		normal_record[i2].num_faces++;

		normal_record[i3].normal_sum.x += normal.x;
		normal_record[i3].normal_sum.y += normal.y;
		normal_record[i3].normal_sum.z += normal.z;
		normal_record[i3].num_faces++;
	}

	/* 3. Distribute the normals over the vertices */
	mesh->num_normals = mesh->num_vertices;
	mesh->normal = calloc(mesh->num_normals, sizeof(Normal));
	for (i = 0; i < mesh->num_vertices; i++)
	{
		double x, y, z, d;
		x = normal_record[i].normal_sum.x;
		y = normal_record[i].normal_sum.y;
		z = normal_record[i].normal_sum.z;
		d = sqrt(SQUARE(x) + SQUARE(y) + SQUARE(z));

		if (normal_record[i].num_faces == 0)
			continue; /* FIXME: What to do about lonely vertices? */

		mesh->normal[i].x = x / d;
		mesh->normal[i].y = y / d;
		mesh->normal[i].z = z / d;
	}

	free(normal_record);
}

/* WARNING: despite the name, the returned normal is not yet
 * normalized */
static Normal calc_face_normal(Vertex p1, Vertex p2, Vertex p3)
{
	Vec3 v1, v2, v3;
	Normal n;

	v1.x = p2.x - p1.x;
	v1.y = p2.y - p1.y;
	v1.z = p2.z - p1.z;

	v2.x = p3.x - p1.x;
	v2.y = p3.y - p1.y;
	v2.z = p3.z - p1.z;

	v3 = vec3_cross(v1, v2);

	n.x = v3.x;
	n.y = v3.y;
	n.z = v3.z;

	return n;
}

/* XXX Split off in renderer.c, remove glew.h */
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
