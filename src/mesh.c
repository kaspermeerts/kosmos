#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <rply.h>

#include "mathlib.h"
#include "mesh.h"

enum vertex_props {
	PROP_X,
	PROP_Y,
	PROP_Z 
};

extern char *strdup(const char *s); /* FIXME */

static bool mesh_load_ply(Mesh *mesh, const char *filename);
static int vertex_cb(p_ply_argument argument);
static int face_cb(p_ply_argument argument);
static void generate_normals(Mesh *mesh);
static Normal calc_face_normal(Vertex v1, Vertex v2, Vertex v3);
static void unitize_mesh(Mesh *mesh);

Mesh *mesh_import(const char *filename)
{
	Mesh *mesh;
	const char *tail;

	mesh = malloc(sizeof(Mesh));

	tail = strrchr(filename, '/');
	/* TODO: Other fileformats */
	if (mesh_load_ply(mesh, filename) == false)
	{
		fprintf(stderr, "Couldn't load model at %s\n", filename);
		free(mesh);
		return NULL;
	}

	mesh->name = strdup((tail ? tail + 1 : filename));
	generate_normals(mesh); /* FIXME: What if we already have normals? */
	unitize_mesh(mesh);

	return mesh;
}

static bool mesh_load_ply(Mesh *mesh, const char *filename)
{
	p_ply ply = NULL;
	p_ply_element element = NULL;
	bool succes = false;

	if ((ply = ply_open(filename, NULL)) == NULL)
		goto out;
	if (ply_read_header(ply) == 0)
		goto out;

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
		} else
		{
			fprintf(stderr, "Unknown element: %s\n", name);
			goto out;
		}
	}

	ply_set_read_cb(ply, "vertex", "x", vertex_cb, mesh, (long) PROP_X);
	ply_set_read_cb(ply, "vertex", "y", vertex_cb, mesh, (long) PROP_Y);
	ply_set_read_cb(ply, "vertex", "z", vertex_cb, mesh, (long) PROP_Z);
	ply_set_read_cb(ply, "face", "vertex_indices", face_cb, mesh, 0);

	if (ply_read(ply) == 1)
		succes = true;

out:
	if (ply != NULL)
		ply_close(ply);
	return succes;
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
		return 0;
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

static void unitize_mesh(Mesh *mesh)
{
	int i;
	GLfloat xmin, ymin, zmin, xmax, ymax, zmax;
	GLfloat scale;

	xmin = ymin = zmin =  1e37;
	xmax = ymax = zmax = -1e37;

	for (i = 0; i < mesh->num_vertices; i++)
	{
		Vertex v = mesh->vertex[i];

		if (xmin > v.x)
			xmin = v.x;
		if (ymin > v.y)
			ymin = v.y;
		if (zmin > v.z)
			zmin = v.z;
		if (xmax < v.x)
			xmax = v.x;
		if (ymax < v.y)
			ymax = v.y;
		if (zmax < v.z)
			zmax = v.z;
	}

	scale = MAX(xmax - xmin, MAX(ymax - ymin, zmax - zmin)) / 2;
	for (i = 0; i < mesh->num_vertices; i++)
	{
		mesh->vertex[i].x = (mesh->vertex[i].x - (xmin + xmax)/2)/scale;
		mesh->vertex[i].y = (mesh->vertex[i].y - (ymin + ymax)/2)/scale;
		mesh->vertex[i].z = (mesh->vertex[i].z - (zmin + zmax)/2)/scale;
	}

	return;
}
