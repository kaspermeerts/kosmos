#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>

#include "linereader.h"
#include "mesh.h"

/* Mesh loading code
 * Ugly but it's the only way */

enum pass_num {
	FIRST_PASS,
	SECOND_PASS
};

static int load_mesh_stream(Mesh *mesh, FILE *stream, enum pass_num);
static int first_pass_line(Mesh *mesh, const char *line);
static int second_pass_line(Mesh *mesh, const char *line);
static int count_triangles(const char *line);
static Vertex parse_vertex(const char *line);
static Normal parse_normal(const char *line);
static TexCoord parse_texcoord(const char *line);
static int parse_triangles(Mesh *mesh, const char *line, int cur_triangle);

int main(int argc, char **argv)
{
	Mesh *mesh;

	if (argc < 2)
		return 1;
	
	mesh = load_mesh(argv[1]);

	return 0;
}

Mesh *load_mesh(const char *filename)
{
	Mesh *mesh;
	FILE *stream;

	stream = fopen(filename, "rb");
	if (stream == NULL)
		return NULL;

	mesh = malloc(sizeof(Mesh));

	load_mesh_stream(mesh, stream, FIRST_PASS);
	mesh->vertex = calloc(mesh->num_vertices, sizeof(Vertex));
	mesh->normal = calloc(mesh->num_normals, sizeof(Normal));
	mesh->texcoord = calloc(mesh->num_texcoords, sizeof(TexCoord));
	mesh->triangle = calloc(mesh->num_triangles, sizeof(Triangle));
	load_mesh_stream(mesh, stream, SECOND_PASS);

	free(mesh);

	return mesh;
}

/* Load the data line by line */
static int load_mesh_stream(Mesh *mesh, FILE *stream, enum pass_num pass)
{
	char *line;
	int ret;
	LineReader lr;

	mesh->num_vertices = mesh->num_normals = mesh->num_texcoords = 0;
	mesh->num_triangles = mesh->num_groups = 0;

	lr = linereader_start(stream);

	while ((line = linereader_get(&lr)) != NULL )
	{
		if (pass == FIRST_PASS)
			ret = first_pass_line(mesh, line);
		else
			ret = second_pass_line(mesh, line);

		if (ret != 0)
			return 1;

	}

	linereader_stop(&lr);

	free(line);

	printf("Mesh statistics:\n");
	printf("%u\tvertices\n", mesh->num_vertices);
	printf("%u\tnormals\n", mesh->num_normals);
	printf("%u\ttexcoords\n", mesh->num_texcoords);
	printf("%u\ttriangles\n", mesh->num_triangles);
	printf("%u\tgroups\n", mesh->num_groups);
	return 0;
}

/*
 * First pass
 */

static int first_pass_line(Mesh *mesh, const char *line)
{
	char tmp[128]; /* XXX Arbitrary, but should be enough for everything */
	int n;

	if (sscanf(line, "%s %n", tmp, &n) == EOF)
		return 0;

	line += n;

	switch (tmp[0])
	{
	case '#': /* Comment */
	case '\0':/* Whitespace */
		break;
	case 'v': /* Vertex, normal or texture coordinate */
		switch(tmp[1])
		{
		case '\0':
			mesh->num_vertices++;
			break;
		case 'n':
			mesh->num_normals++;
			break;
		case 't':
			mesh->num_texcoords++;
			break;
		default:
			printf("Malformed line: %s\n", line);
			return 1;
			break;
		}
		break;
	case 'f': /* Face */
		mesh->num_triangles += count_triangles(line);
		break;
	case 'g': /* Group */
		mesh->num_groups++;
		break;
	case 'o': /* Object */
		break;
	default:
		printf("Malformed line: %s\n", line);
		return 1;
		break;
	}

	return 0;
}

static int count_triangles(const char *line)
{
	const char *start;
	int num = 0, n, t; /* t is for throwaway values */

	/* There are four possibilities: v//n, v/t/n, v/t, v */
	if ((start = strstr(line, "//")) != NULL)
	{ /* v//n */
		start += 2;
		if ((start = strstr(start, "//")) == NULL)
			return 0;
		start += 2;
		if ((start = strstr(start, "//")) == NULL)
			return 0;
		start += 2;

		num = 1;
		while ((start = strstr(start, "//")))
		{
			start += 2;
			num++;
		}

		return num;
	} else if ((start = strchr(line, '/')) != NULL)
	{
		start++;
		if ((start = strchr(start, '/')) != NULL)
		{ /* v/t/n */
			if (sscanf(line, "%d/%d/%d %d/%d/%d %d/%d/%d  %n\n",
			                  &t,&t,&t,&t,&t,&t,&t,&t,&t, &n) < 9)
				return 0;

			num = 1;
			start = line + n;
			while (sscanf(start, "%d/%d/%d %n", &t, &t, &t, &n) == 3)
			{
				num++;
				start += n;
			}
		} else
		{ /* v/t */
			if (sscanf(line, "%d/%d %d/%d %d/%d  %n\n", 
			                  &t,&t,&t,&t,&t,&t, &n) < 6)
				return 0;

			num = 1;
			start = line + n;
			while (sscanf(start, "%d/%d %n", &n, &n, &n) == 2)
			{
				num++;
				start += n;
			}
			
		}
	} else
	{ /* v */
		if (sscanf(line, "%d %d %d %n", &n, &n, &n, &n) < 3)
			return 0;

		num = 1;
		start = line + n;
		while (sscanf(start, "%d %n", &n, &n) == 1)
		{
			start += n;
			num++;
		}
	}

	return num;
}

/***************
 * Second pass *
 ***************/
static int second_pass_line(Mesh *mesh, const char *line)
{
	char tmp[128]; /* XXX Arbitrary, but should be enough for everything */
	int n;
	static int cur_vert = 0;
	static int cur_normal = 0;
	static int cur_texcoord = 0;
	static int cur_triangle = 0;

	if (sscanf(line, "%s %n", tmp, &n) == EOF)
		return 0;

	line += n;

	switch (tmp[0])
	{
	case '#': /* Comment */
	case '\0':/* Whitespace */
		break;
	case 'v': /* Vertex, normal or texture coordinate */
		switch(tmp[1])
		{
		case '\0':
			mesh->vertex[cur_vert] = parse_vertex(line);
			cur_vert++;
			break;
		case 'n':
			mesh->normal[cur_normal] = parse_normal(line);
			cur_normal++;
			break;
		case 't':
			mesh->texcoord[cur_texcoord] = parse_texcoord(line);
			cur_texcoord++;
			break;
		default:
			printf("Malformed line: %s\n", line);
			return 1;
			break;
		}
		break;
	case 'f': /* Face */
		cur_triangle += parse_triangles(mesh, line, cur_triangle);
		break;
	case 'g': /* Group */
		break;
	case 'o': /* Object */
		break;
	default:
		printf("Malformed line: %s\n", line);
		return 1;
		break;
	}

	return 0;
}

static Vertex parse_vertex(const char *line)
{
	GLfloat x, y, z;
	Vertex item;

	sscanf(line, " v %f %f %f\n", &x, &y, &z);
	item.x = x;
	item.y = y;
	item.z = z;

	return item;
}

static Normal parse_normal(const char *line)
{
	GLfloat x, y, z;
	Normal item;

	sscanf(line, " vn %f %f %f\n", &x, &y, &z);
	item.x = x;
	item.y = y;
	item.z = z;

	return item;
}

static TexCoord parse_texcoord(const char *line)
{
	GLfloat u, v;
	TexCoord item;

	sscanf(line, " vt %f %f\n", &u, &v);
	item.u = u;
	item.v = v;

	return item;
}

static int parse_triangles(Mesh *mesh, const char *line, int cur_triangle)
{
	const char *start;
	int n;

	/* There are four possibilities: v//n, v/t/n, v/t, v */
	if (strstr(line, "//") != NULL)
	{ /* v//n */
		if (sscanf(" f %d//%d %d//%d %d//%d %n", 
				&v1, &n1, &v2, &n2, &v3, &n3, &n) < 6)
				return 0;
	} else if (strchr(line, '/') != NULL)
	{
		return 0;
	}

}

