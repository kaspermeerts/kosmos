#include <stdio.h>
#include <stdlib.h>
#include <ralloc.h>

#include "mesh.h"

static void mesh_print(Mesh *mesh);

static void mesh_print(Mesh *mesh)
{
	printf("Mesh %s\n", mesh->name);
	printf("%d Vertices\n", mesh->num_vertices);
	printf("%d Normals\n", mesh->num_normals);
	printf("%d Triangles\n", mesh->num_indices / 3);
}

int main(int argc, char **argv)
{
	const char *filename;
	Mesh *mesh;
	
	if (argc < 2)
	{
		fprintf(stderr, "Give a model filename\n");
		return 1;
	}

	filename = argv[1];

	mesh = mesh_import(filename);
	if (mesh == NULL)
		return 1;

	mesh_print(mesh);

	ralloc_free(mesh);

	return 0;
}


