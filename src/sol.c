#include <stdlib.h>
#include <stdio.h>
#include <ralloc.h>

#include "solarsystem.h"

#define XSTRINGIFY(s) #s
#define STRINGIFY(x) XSTRINGIFY(x)

int main(int argc, char **argv)
{
	SolarSystem *sol;
	const char *filename;

	if (argc < 2)
		filename = STRINGIFY(ROOT_PATH) "/data/sol.ini";
	else
		filename = argv[1];
	
	sol = solsys_load(filename);
	if (sol == NULL)
		return 1;
	
	for (int i = 0; i < sol->num_bodies; i++)
	{
		Body *body = &sol->body[i];
		printf("Name: %s\n", body->name);
		printf("Primary: %s\n", body->primary ? body->primary->name : "(none)");
		printf("%d satellites:\n", body->num_satellites);
		for (int j = 0; j < body->num_satellites; j++)
			printf("%s\n", body->satellite[j]->name);
		printf("\n");
	}

	ralloc_free(sol);

	return 0;
}	
