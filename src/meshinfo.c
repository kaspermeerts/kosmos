#include <stdio.h>
#include <stdlib.h>

#include "mesh.h"

void mesh_print(Mesh *mesh)
{
	printf("Mesh %s\n", mesh->name);
	printf("%d Vertices\n", mesh->num_vertices);
	printf("%d Normals\n", mesh->num_normals);
	printf("%d Triangles\n", mesh->num_triangles);
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

	free(mesh->name);
	free(mesh->normal);
	free(mesh->vertex);
	free(mesh->triangle);
	free(mesh);

	return 0;
}


