#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>

#include "solarsystem.h"

#define STRINGIFY(s) XSTRINGIFY(s)
#define XSTRINGIFY(s) #s

/* FIXME: Memory leaks à volonté. First work is using a recursive memory
 * allocator to fix this. */
extern char *strdup(const char *str); /* FIXME Damnit */

static bool config_get_long(ALLEGRO_CONFIG *cfg, const char *sec,
		const char *key, long *ret);
static bool config_get_double(ALLEGRO_CONFIG *cfg, const char *sec,
		const char *key, double *ret);
static bool config_get_string(ALLEGRO_CONFIG *cfg, const char *sec,
		const char *key, char **ret);
static bool load_planet(ALLEGRO_CONFIG *cfg, const char *sec, 
		Planet *planet);
static SolarSystem *load_from_config(ALLEGRO_CONFIG *cfg);

static bool config_get_long(ALLEGRO_CONFIG *cfg, const char *sec,
		const char *key, long *ret)
{
	int saved_errno;
	const char *value;
	char *endptr;

	value = al_get_config_value(cfg, sec, key);
	if (value == NULL)
	{
		fprintf(stderr, "Error in section %s: Key %s not found\n", sec, key);
		return false;
	}
	if (value[0] == '\0')
	{
		fprintf(stderr, "Error in section %s: Key %s empty\n", sec, key);
		return false;
	}

	saved_errno = errno;
	errno = 0;
	*ret = strtol(value, &endptr, 10);
	if (errno != 0 || *endptr != '\0')
	{
		/* TODO: Add strerror() */
		fprintf(stderr, "Error in section %s: Key %s invalid\n", sec, key);
		return false;
	}
	errno = saved_errno;
	if (endptr == value)
	{
		fprintf(stderr, "Error in section %s: Key %s empty\n", sec, key);
		return false;
	}


	return true;
}

static bool config_get_double(ALLEGRO_CONFIG *cfg, const char *sec,
		const char *key, double *ret)
{
	int saved_errno;
	const char *value;
	char *endptr;

	value = al_get_config_value(cfg, sec, key);
	if (value == NULL)
	{
		fprintf(stderr, "Error in section %s: Key %s not found\n", sec, key);
		return false;
	}

	saved_errno = errno;
	errno = 0;
	*ret = strtod(value, &endptr);
	if (errno != 0 || *endptr != '\0')
	{
		/* TODO: Add strerror() */
		fprintf(stderr, "Error in section %s: Key %s invalid\n", sec, key);
		return false;
	}
	errno = saved_errno;
	if (endptr == value)
	{
		fprintf(stderr, "Error in section %s: Key %s empty\n", sec, key);
		return false;
	}


	return true;
}

static bool config_get_string(ALLEGRO_CONFIG *cfg, const char *sec,
		const char *key, char **ret)
{
	const char *value;
	char *dupstr;

	value = al_get_config_value(cfg, sec, key);
	if (value == NULL)
	{
		fprintf(stderr, "Error in section %s: Key %s not found\n", sec, key);
		return false;
	}
	if (value[0] == '\0')
	{
		fprintf(stderr, "Error in section %s: Key %s empty\n", sec, key);
		return false;
	}

	dupstr = strdup(value);
	if (dupstr == NULL)
	{
		fprintf(stderr, "Out of memory\n");
		return false;
	}

	*ret = dupstr;

	return true;
}
static bool load_planet(ALLEGRO_CONFIG *cfg, const char *sec, 
		Planet *planet)
{
	bool success;
	
	if (al_get_first_config_entry(cfg, sec, NULL) == NULL)
	{
		fprintf(stderr, "No section for %s\n", sec);
		return false;
	}

	success = config_get_string(cfg, sec, "Name", &planet->name);
	if (!success)
	{
		fprintf(stderr, "Planet name derp\n"); /* FIXME: I need a decent logging system. This just sucks */
		return false;
	}

	success = config_get_double(cfg, sec, "Mass", &planet->mass);
	if (!success)
	{
		fprintf(stderr, "Planet mass derp\n");
		return false;
	}

	if (!config_get_double(cfg, sec, "Ecc", &planet->orbit.Ecc) ||
	    !config_get_double(cfg, sec, "SMa", &planet->orbit.SMa) ||
	    !config_get_double(cfg, sec, "Inc", &planet->orbit.Inc) ||
	    !config_get_double(cfg, sec, "LAN", &planet->orbit.LAN) ||
	    !config_get_double(cfg, sec, "APe", &planet->orbit.APe) ||
	    !config_get_double(cfg, sec, "MnA", &planet->orbit.MnA))
	{
		fprintf(stderr, "Derp loading the orbital elements\n");
		return false;
	}

	return true;
}

static SolarSystem *load_from_config(ALLEGRO_CONFIG *cfg)
{
	SolarSystem *solsys;
	long num_planets;
	char secname[16]; /* Enough for 10,000,000 planets */
	bool success = true;
	int i;

	success = config_get_long(cfg, "Star", "Planets", &num_planets);
	if (!success)
	{
		fprintf(stderr, "No number of planets in file\n");
		return NULL;
	}
	if (num_planets < 0)
	{
		fprintf(stderr, "Number of planets must be positive\n");
		return NULL;
	}

	solsys = malloc(sizeof(SolarSystem) + num_planets*sizeof(Planet));
	if (solsys == NULL)
	{
		fprintf(stderr, "Out of memory\n");
		return NULL;
	}
	solsys->num_planets = num_planets;

	success = config_get_string(cfg, "Star", "Name", &solsys->name);
	if (!success)
	{
		fprintf(stderr, "Star has no name\n");
		free(solsys);
		return NULL;
	}

	success = config_get_double(cfg, "Star", "Mass", &solsys->star_mass);
	if (!success)
	{
		fprintf(stderr, "No mass given\n");
		free(solsys->name);
		free(solsys);
		return NULL;
	}

	success = config_get_double(cfg, "Star", "Gravitational parameter", 
			&solsys->star_mu);
	if (!success)
		solsys->star_mu = GRAV_CONST * solsys->star_mass;

	for (i = 0; i < solsys->num_planets; i++)
	{
		Planet *planet = &solsys->planet[i];

		snprintf(secname, sizeof(secname), "Planet %d", i + 1);
		
		if (load_planet(cfg, secname, planet) == false)
		{
			fprintf(stderr, "Failure to load planet %d\n", i + 1);
			free(solsys->name);
			free(solsys);
			return NULL;
		}

		planet->orbit.period = M_TWO_PI * 
				sqrt(CUBE(planet->orbit.SMa) / solsys->star_mu);
	}	

	return solsys;
}

SolarSystem *solsys_load(const char *filename)
{
	SolarSystem *ret;
	ALLEGRO_FILE *file;
	ALLEGRO_CONFIG *cfg;
	int i;

	file = al_fopen(filename, "rb");
	if (file == NULL)
	{
		fprintf(stderr, "Error opening file %s\n", filename);
		return NULL;
	}

	cfg = al_load_config_file_f(file);
	if (cfg == NULL)
	{
		al_fclose(file);
		fprintf(stderr, "Not a valid config file\n");
		return NULL;
	}

	ret = load_from_config(cfg);

	printf("Finished loading a solarsystem with %d planets\n", ret->num_planets);
	printf("%s", ret->planet[0].name);
	for (i = 1; i < ret->num_planets; i++)
		printf(", %s", ret->planet[i].name);
	printf("\n");
	
	al_destroy_config(cfg);
	al_fclose(file);

	return ret;
}

int main(int argc, char **argv)
{
	char *filename;

	if (argc < 2)
		filename = STRINGIFY(ROOT_PATH) "/data/sol.ini";
	else
		filename = argv[1];

	solsys_load(filename);

	return 0;
}
