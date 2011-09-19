#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>

#include "log.h"
#include "solarsystem.h"

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
		log_err("Section %s: Key %s not found\n", sec, key);
		return false;
	}
	if (value[0] == '\0')
	{
		log_err("Section %s: Key %s empty\n", sec, key);
		return false;
	}

	saved_errno = errno;
	errno = 0;
	*ret = strtol(value, &endptr, 10);
	if (errno != 0)
	{
		log_err("Section %s: Key %s invalid: %s\n", sec, key,
				strerror(errno));
		return false;
	}
	errno = saved_errno;
	
	if (*endptr != '\0')
	{
		log_err("Section %s: Key %s invalid\n", sec, key);
		return false;
	}
	if (endptr == value)
	{
		log_err("Section %s: Key %s empty\n", sec, key);
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
		log_err("Section %s: Key %s not found\n", sec, key);
		return false;
	}

	saved_errno = errno;
	errno = 0;
	*ret = strtod(value, &endptr);
	if (errno != 0)
	{
		log_err("Section %s: Key %s invalid: %s\n", sec, key,
				strerror(errno));
		return false;
	}
	errno = saved_errno;
	
	if (*endptr != '\0')
	{
		log_err("Section %s: Key %s invalid\n", sec, key);
		return false;
	}
	if (endptr == value)
	{
		log_err("Section %s: Key %s empty\n", sec, key);
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
		log_err("Error in section %s: Key %s not found\n", sec, key);
		return false;
	}
	if (value[0] == '\0')
	{
		log_err("Error in section %s: Key %s empty\n", sec, key);
		return false;
	}

	dupstr = strdup(value);
	if (dupstr == NULL)
	{
		log_err("Out of memory\n");
		return false;
	}

	*ret = dupstr;

	return true;
}

static bool load_planet(ALLEGRO_CONFIG *cfg, const char *sec, 
		Planet *planet)
{
	if (al_get_first_config_entry(cfg, sec, NULL) == NULL)
	{
		log_err("Section %s not in file\n", sec);
		return false;
	}

	if (!config_get_string(cfg, sec, "Name", &planet->name))
	{
		log_err("Couldn't find %s's name\n", sec);
		return false;
	}

	if (!config_get_double(cfg, sec, "Mass", &planet->mass))
	{
		log_err("Couldn't find %s's mass\n", sec);
		return false;
	}

	if (!config_get_double(cfg, sec, "Ecc", &planet->orbit.Ecc) ||
	    !config_get_double(cfg, sec, "SMa", &planet->orbit.SMa) ||
	    !config_get_double(cfg, sec, "Inc", &planet->orbit.Inc) ||
	    !config_get_double(cfg, sec, "LAN", &planet->orbit.LAN) ||
	    !config_get_double(cfg, sec, "APe", &planet->orbit.APe) ||
	    !config_get_double(cfg, sec, "MnA", &planet->orbit.MnA))
	{
		log_err("Couldn't load orbital elements of %s\n", sec);
		return false;
	}

	return true;
}

static SolarSystem *load_from_config(ALLEGRO_CONFIG *cfg)
{
	SolarSystem *solsys;
	long num_planets;
	char secname[16]; /* Enough for 10,000,000 planets */
	int i;

	if (!config_get_long(cfg, "Star", "Planets", &num_planets))
	{
		log_err("No number of planets in file\n");
		return NULL;
	}
	if (num_planets < 0)
	{
		log_err("Number of planets must be positive\n");
		return NULL;
	}

	solsys = malloc(sizeof(SolarSystem) + num_planets*sizeof(Planet));
	if (solsys == NULL)
	{
		log_err("Out of memory\n");
		return NULL;
	}
	solsys->num_planets = num_planets;

	if (!config_get_string(cfg, "Star", "Name", &solsys->name))
	{
		log_err("Star has no name\n");
		free(solsys);
		return NULL;
	}

	if (!config_get_double(cfg, "Star", "Mass", &solsys->star_mass))
	{
		log_err("No mass given\n");
		free(solsys->name);
		free(solsys);
		return NULL;
	}

	if (!config_get_double(cfg, "Star", "Gravitational parameter", 
			&solsys->star_mu))
		solsys->star_mu = GRAV_CONST * solsys->star_mass;

	for (i = 0; i < solsys->num_planets; i++)
	{
		Planet *planet = &solsys->planet[i];

		snprintf(secname, sizeof(secname), "Planet %d", i + 1);
		
		if (load_planet(cfg, secname, planet) == false)
		{
			log_err("Failure to load planet %d\n", i + 1);
			free(solsys->name);
			free(solsys);
			return NULL;
		}

		/* XXX Is this the best place to set this? */
		planet->orbit.epoch = 0;
		planet->orbit.period = M_TWO_PI * 
				sqrt(CUBE(planet->orbit.SMa) / solsys->star_mu);
		mat3_euler(RAD(planet->orbit.LAN), 
				RAD(planet->orbit.Inc),
				RAD(planet->orbit.APe),
				planet->orbit.plane_orientation);

		/*
		planet->orbit_path = calloc(sizeof(Vec3), planet->num_samples);
		for (j = 0; j < planet->num_samples; j++)
			planet->orbit_path[j] = kepler_position_at_true_anomaly(
					&planet->orbit, M_TWO_PI / planet->num_samples * j);	
		*/
	}	

	return solsys;
}

SolarSystem *solsys_load(const char *filename)
{
	SolarSystem *ret;
	ALLEGRO_FILE *file;
	ALLEGRO_CONFIG *cfg;

	file = al_fopen(filename, "rb");
	if (file == NULL)
	{
		log_err("Error opening file %s\n", filename);
		return NULL;
	}

	cfg = al_load_config_file_f(file);
	if (cfg == NULL)
	{
		al_fclose(file);
		log_err("Not a valid config file\n");
		return NULL;
	}

	ret = load_from_config(cfg);

	if (ret == NULL)
		log_err("Couldn't load solarsystem from file: %s\n", filename);
	else
		log_dbg("Loaded a solarsystem with %d planets\n", ret->num_planets);
	
	al_destroy_config(cfg);
	al_fclose(file);

	return ret;
}
