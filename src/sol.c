#include <stdlib.h>
#include <stdio.h>
#include <ralloc.h>

#include "solarsystem.h"

#define XSTRINGIFY(s) #s
#define STRINGIFY(x) XSTRINGIFY(x)

static void print_satellites(int level, Body *body)
{
	int i, j;

	for (i = 0; i < body->num_satellites; i++)
	{
		for (j = 0; j < level; j++)
			printf("    ");
		printf("%s\n", body->satellite[i]->name);
		print_satellites(level + 1, body->satellite[i]);
	}
}

int main(int argc, char **argv)
{
	SolarSystem *sol;
	Body *center;
	const char *filename;

	if (argc < 2)
		filename = STRINGIFY(ROOT_PATH) "/data/sol.ini";
	else
		filename = argv[1];
	
	sol = solsys_load(filename);
	if (sol == NULL)
		return 1;
	
	for (int i = 0; i < sol->num_bodies; i++)
		if (sol->body[i].primary == NULL)
			center = &sol->body[i];
	
	printf("%s\n", center->name);
	print_satellites(1, center);

	ralloc_free(sol);

	return 0;
}	
