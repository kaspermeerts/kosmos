#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <ralloc.h>
#include <rply.h>
#include <allegro5/allegro.h>

#include "mathlib.h"
#include "mesh.h"
#include "log.h"

enum vertex_props {
	PROP_X,
	PROP_Y,
	PROP_Z 
};

struct face_types {
	bool all_triangles;
	bool all_quads;
};

static bool mesh_load_ply(Mesh *mesh, const char *filename);
static bool mesh_ply_first_pass(Mesh *mesh, p_ply ply);
static bool mesh_ply_second_pass(Mesh *mesh, p_ply ply);
static int tesselate_cb(p_ply_argument argument);
static int vertex_cb(p_ply_argument argument);
static int face_cb(p_ply_argument argument);
static void generate_normals(Mesh *mesh);
static Normal calc_tri_normal(Vertex v1, Vertex v2, Vertex v3);
static Normal calc_quad_normal(Vertex v1, Vertex v2, Vertex v3, Vertex v4);

Mesh *mesh_import(const char *filename)
{
	ALLEGRO_PATH *path;
	Mesh *mesh;

	if ((mesh = ralloc(NULL, Mesh)) == NULL)
		return NULL;

	/* TODO: Other fileformats */
	if (mesh_load_ply(mesh, filename) == false)
	{
		log_err("Couldn't load model at %s\n", filename);
		ralloc_free(mesh);
		return NULL;
	}

	path = al_create_path(filename);
	mesh->name = ralloc_strdup(mesh, al_get_path_filename(path));
	log_dbg("Loaded mesh %s\n", mesh->name);
	al_destroy_path(path);
	generate_normals(mesh); /* FIXME: What if we already have normals? */
	mesh_unitize(mesh);

	return mesh;
}

static bool mesh_load_ply(Mesh *mesh, const char *filename)
{
	p_ply ply;

	if ((ply = ply_open(filename, NULL, 0, NULL)) == NULL)
		goto errorout;
	if (ply_read_header(ply) == 0)
		goto errorout;

	if (!mesh_ply_first_pass(mesh, ply))
		goto errorout;

	ply_close(ply);

	if ((ply = ply_open(filename, NULL, 0, NULL)) == NULL)
		goto errorout;
	if (ply_read_header(ply) == 0)
		goto errorout;

	if(!mesh_ply_second_pass(mesh, ply))
		goto errorout;

	ply_close(ply);
	
	return true;
errorout:
	if (ply != NULL)
		ply_close(ply);
	return false;
}

static bool mesh_ply_first_pass(Mesh *mesh, p_ply ply)
{
	struct face_types types;
	long num_faces;

	/* First pass */
	types.all_triangles = true;
	types.all_quads = true;

	num_faces = ply_set_read_cb(ply, "face", "vertex_indices", tesselate_cb, &types, 0);

	if (ply_read(ply) != 1)
		return false;
	if (types.all_triangles)
	{
		mesh->type = GL_TRIANGLES;
		mesh->num_indices = num_faces * 3;
	} else if (types.all_quads)
	{
		mesh->type = GL_QUADS;
		mesh->num_indices = num_faces * 4;
	} else
	{
		log_err("Inconsistent number of vertices per face\n");
		return false;
	}

	return true;
}

static bool mesh_ply_second_pass(Mesh *mesh, p_ply ply)
{
	long num_vertices;

	num_vertices = 
	ply_set_read_cb(ply, "vertex", "x", vertex_cb, mesh, (long) PROP_X);
	ply_set_read_cb(ply, "vertex", "y", vertex_cb, mesh, (long) PROP_Y);
	ply_set_read_cb(ply, "vertex", "z", vertex_cb, mesh, (long) PROP_Z);
	ply_set_read_cb(ply, "face", "vertex_indices", face_cb, mesh, 0);

	mesh->index = ralloc_array(mesh, GLuint, mesh->num_indices);
	mesh->num_vertices = num_vertices;
	mesh->vertex = ralloc_array(mesh, Vertex, mesh->num_vertices);

	if (ply_read(ply) != 1)
		return false;
	return true;
}

static int tesselate_cb(p_ply_argument argument)
{
	struct face_types *types;
	void *pdata;
	long len;

	ply_get_argument_user_data(argument, &pdata, NULL);
	ply_get_argument_property(argument, NULL, &len, NULL);

	types = (struct face_types *)pdata;

	if (len != 3)
		types->all_triangles = false;
	if (len != 4)
		types->all_quads = false;

	return 1;
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
		mesh->vertex[index].x = (GLfloat) data;
		break;
	case PROP_Y:
		mesh->vertex[index].y = (GLfloat) data;
		break;
	case PROP_Z:
		mesh->vertex[index].z = (GLfloat) data;
		break;
	default:
		/* Shouldn't happen */
		log_err("Internal consistency error\n");
		return 0;
		break;
	}

	return 1;
}

static int face_cb(p_ply_argument argument)
{
	Mesh *mesh;
	void *pdata;
	long index, len, value_index, nvert;
	
	ply_get_argument_user_data(argument, &pdata, NULL);
	mesh = (Mesh *)pdata;
	if (mesh->type == GL_TRIANGLES)
		nvert = 3;
	else if (mesh->type == GL_QUADS)
		nvert = 4;
	else
		nvert = 0; /* Shut up GCC */

	ply_get_argument_element(argument, NULL, &index);
	ply_get_argument_property(argument, NULL, &len, &value_index);
	
	if (len != nvert)
	{
		/* TODO: Tesselate? */
		log_err("Malformed face\n");
		return 0;
	}
	
	if (value_index == -1)
		return 1;

	mesh->index[nvert*index + value_index] = ply_get_argument_value(argument);

	return 1;
}

/* Generate normals based on the geometry of the mesh. */
static void generate_normals(Mesh *mesh)
{
	struct {
		int num_faces;
		Normal normal_sum;
	} *normal_record;
	int i;

	/* 1. Initialize all normal sums to zero */
	normal_record = rzalloc_array_size(NULL, sizeof(*normal_record),
			mesh->num_vertices);

	/* 2. Accumulate the normals of each face onto the vertices */
	if (mesh->type == GL_TRIANGLES)
	{
		for (i = 0; i < mesh->num_indices; i+=3)
		{
			Normal normal;
			GLuint i1, i2, i3;

			i1 = mesh->index[i + 0];
			i2 = mesh->index[i + 1];
			i3 = mesh->index[i + 2];

			normal = calc_tri_normal(mesh->vertex[i1], mesh->vertex[i2], 
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
	}
	else if (mesh->type == GL_QUADS)
	{
		for (i = 0; i < mesh->num_indices; i+=4)
		{
			Normal normal;
			GLuint i1, i2, i3, i4;

			i1 = mesh->index[i + 0];
			i2 = mesh->index[i + 1];
			i3 = mesh->index[i + 2];
			i4 = mesh->index[i + 3];

			normal = calc_quad_normal(mesh->vertex[i1], mesh->vertex[i2], 
					mesh->vertex[i3], mesh->vertex[i4]);

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

			normal_record[i4].normal_sum.x += normal.x;
			normal_record[i4].normal_sum.y += normal.y;
			normal_record[i4].normal_sum.z += normal.z;
			normal_record[i4].num_faces++;
		}
	}

	/* 3. Distribute the normals over the vertices */
	mesh->num_normals = mesh->num_vertices;
	mesh->normal = ralloc_array(mesh, Normal, mesh->num_normals);
	for (i = 0; i < mesh->num_vertices; i++)
	{
		double x, y, z, d;
		x = normal_record[i].normal_sum.x;
		y = normal_record[i].normal_sum.y;
		z = normal_record[i].normal_sum.z;
		d = sqrt(SQUARE(x) + SQUARE(y) + SQUARE(z));

		if (normal_record[i].num_faces == 0)
			continue; /* FIXME: What to do about lonely vertices? */

		mesh->normal[i].x = (GLfloat) (x / d);
		mesh->normal[i].y = (GLfloat) (y / d);
		mesh->normal[i].z = (GLfloat) (z / d);
	}

	ralloc_free(normal_record);
}

/* WARNING: despite the name, the returned normal is not yet
 * normalized */
static Normal calc_tri_normal(Vertex p1, Vertex p2, Vertex p3)
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

	n.x = (GLfloat) v3.x;
	n.y = (GLfloat) v3.y;
	n.z = (GLfloat) v3.z;

	return n;
}

static Normal calc_quad_normal(Vertex p1, Vertex p2, Vertex p3, Vertex p4)
{
	Vec3 v1, v2, v3;
	Normal n;

	v1.x = p3.x - p1.x;
	v1.y = p3.y - p1.y;
	v1.z = p3.z - p1.z;

	v2.x = p4.x - p2.x;
	v2.y = p4.y - p2.y;
	v2.z = p4.z - p2.z;

	v3 = vec3_cross(v1, v2);

	n.x = (GLfloat) v3.x;
	n.y = (GLfloat) v3.y;
	n.z = (GLfloat) v3.z;

	return n;
}

void mesh_unitize(Mesh *mesh)
{
	int i;
	GLfloat xmin, ymin, zmin, xmax, ymax, zmax;
	GLfloat scale;

	xmin = ymin = zmin =  HUGE_VALF;
	xmax = ymax = zmax = -HUGE_VALF;

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
