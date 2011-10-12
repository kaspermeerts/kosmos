#include <stdio.h>
#include <stdlib.h>
#include <ralloc.h>

#include "mesh.h"

int main(int argc, char **argv)
{
	Mesh *mesh;

	if (argc < 2)
	{
		fprintf(stderr, "Give a model filename\n");
		return 1;
	}

	mesh = mesh_import(argv[1]);
	if (mesh == NULL)
		return 1;

	printf("Mesh %s\n", mesh->name);
	printf("%d Vertices\n", mesh->num_vertices);
	printf("%d Triangles\n", mesh->num_indices / 3);
	printf("%d\n", mesh->type);


	ralloc_free(mesh);

	return 0;
}


