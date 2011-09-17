#ifndef KOSMOS_SOLARSYSTEM_H
#define KOSMOS_SOLARSYSTEM_H

#include "mathlib.h"
#include "keplerorbit.h"

typedef struct Planet {
	char *name;
	double mass;
	KeplerOrbit orbit;
} Planet;

typedef struct SolarSystem {
	char *name;
	double star_mass;
	double star_mu; /* Gravitational parameter */

	int num_planets;
	Planet planet[];
} SolarSystem;

SolarSystem *solsys_load(const char *filename);

#endif
