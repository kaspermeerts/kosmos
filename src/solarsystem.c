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

static bool load_body(ALLEGRO_CONFIG *cfg, const char *sec,	Body *body);
static SolarSystem *load_from_config(ALLEGRO_CONFIG *cfg);

#if 0
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
#endif

static bool config_get_double(ALLEGRO_CONFIG *cfg, const char *sec,
		const char *key, double *ret, bool optional)
{
	int saved_errno;
	const char *value;
	char *endptr;

	value = al_get_config_value(cfg, sec, key);
	if (value == NULL)
	{
		if (!optional)
			log_err("Section %s: Key %s not found\n", sec, key);
		return false;
	}

	saved_errno = errno;
	errno = 0;
	*ret = strtod(value, &endptr);
	if (errno != 0)
	{
		if (!optional)
			log_err("Section %s: Key %s invalid: %s\n", sec, key, 
					strerror(errno));
		return false;
	}
	errno = saved_errno;

	if (*endptr != '\0')
	{
		if (!optional)
			log_err("Section %s: Key %s invalid\n", sec, key);
		return false;
	}
	if (endptr == value)
	{
		if (!optional)
			log_err("Section %s: Key %s empty\n", sec, key);
		return false;
	}

	return true;
}

static bool config_get_string(ALLEGRO_CONFIG *cfg, const char *sec,
		const char *key, char **ret, bool optional)
{
	const char *value;

	*ret = NULL;
	value = al_get_config_value(cfg, sec, key);
	if (value == NULL)
	{
		if (!optional)
			log_err("Error in section %s: Key %s not found\n", sec, key);
		return false;
	}
	if (value[0] == '\0')
	{
		if (!optional)
			log_err("Error in section %s: Key %s empty\n", sec, key);
		return false;
	}

	*ret = strdup(value);
	if (*ret == NULL)
	{
		log_err("Out of memory\n");
		return false;
	}

	return true;
}

static bool load_body(ALLEGRO_CONFIG *cfg, const char *fullname, Body *body)
{
	const char *name;
	char *type;

	if (al_get_first_config_entry(cfg, fullname, NULL) == NULL)
	{
		log_err("Section %s not in file\n", fullname);
		return false;
	}

	if (!config_get_double(cfg, fullname, "Mass", &body->mass, false))
		return false;

	if (!config_get_double(cfg, fullname, "Gravitational parameter",
			&body->grav_param, true))
	{
		body->grav_param = GRAV_CONST * body->mass;
	}

	/* Figure out what kind of object it is */
	config_get_string(cfg, fullname, "Type", &type, true);
	if (type == NULL)
		body->type = BODY_UNKNOWN;
	else if (strcmp(type, "Star") == 0)
		body->type = BODY_STAR;
	else if (strcmp(type, "Planet") == 0)
		body->type = BODY_PLANET;
	else if (strcmp(type, "Moon") == 0)
		body->type = BODY_PLANET;
	else if (strcmp(type, "Comet") == 0)
		body->type = BODY_COMET;
	else
	{
		log_err("Unknown type: %s\n", type);
		return false;
	}

	if ((name = strrchr(fullname, '/')) == NULL)
	{
		body->name = strdup(fullname);
		body->type = (body->type == BODY_UNKNOWN ? BODY_STAR : body->type);
		body->primary = NULL;
		body->primary_name = NULL;
	} else if (name == fullname) /* No primary name, eg: sec = "/Earth" */
	{
		log_err("Missing primary of %s", fullname);
		return false;
	} else
	{
		body->name = strdup(name + 1);
		body->type = (body->type == BODY_UNKNOWN ? BODY_PLANET : body->type);
		body->primary_name = malloc(name - fullname + 1);
		memcpy(body->primary_name, fullname, name - fullname);
		body->primary_name[name - fullname] = '\0';
	}
	
	if (body->primary_name == NULL)
		return true;
	
	if (!config_get_double(cfg, fullname, "Ecc", &body->orbit.Ecc, false) ||
	    !config_get_double(cfg, fullname, "SMa", &body->orbit.SMa, false) ||
	    !config_get_double(cfg, fullname, "Inc", &body->orbit.Inc, false) ||
	    !config_get_double(cfg, fullname, "LAN", &body->orbit.LAN, false) ||
	    !config_get_double(cfg, fullname, "APe", &body->orbit.APe, false) ||
	    !config_get_double(cfg, fullname, "MnA", &body->orbit.MnA, false))
	{
		log_err("Couldn't load orbital elements of %s\n", fullname);
		return false;
	}

	return true;
}

static SolarSystem *load_from_config(ALLEGRO_CONFIG *cfg)
{
	SolarSystem *solsys;
	const char *name;
	long num_bodies;
	ALLEGRO_CONFIG_SECTION *sec;
	int i;

	/* First pass: Determine the number of bodies in the file */
	num_bodies = 0;
	name = al_get_first_config_section(cfg, &sec);
	while (name)
	{
		if (name[0] != '\0')
			num_bodies++;
		name = al_get_next_config_section(&sec);
	}
	
	if (num_bodies == 0)
		return NULL; /* Empty solarsystem */

	solsys = malloc(sizeof(SolarSystem) + num_bodies*sizeof(Body));
	if (solsys == NULL)
		return NULL;
	solsys->num_bodies = num_bodies;

	/* Second pass: Load all celestial bodies */
	if ((name = al_get_first_config_section(cfg, &sec)) == NULL)
	{
		log_err("Internal consistency error\n");
		free(solsys);
		return NULL;
	}

	i = -1;
	while (++i < num_bodies && name)
	{
		Body *body = &solsys->body[i];

		if (name[0] == '\0')
		{
			name = al_get_next_config_section(&sec);
			i--;
			continue;
		}

		if (load_body(cfg, name, body) == false)
		{
			log_err("Couldn't load body %s\n", name);
			free(solsys);
			return NULL;
		}
		name = al_get_next_config_section(&sec);
	}

	if (i < num_bodies)
		log_err("Internal consistency error\n");

	/* Third pass: Connect each body to its primary */
	for (i = 0; i < num_bodies; i++)
	{
		Body *body = &solsys->body[i];
		if (body->primary_name == NULL) /* Independent body */
			continue;
		for (int j = 0; j < num_bodies; j++)
		{
			if (strcmp(body->primary_name, solsys->body[j].name) == 0)
			{
				body->primary = &solsys->body[j];
				break;
			}
		}
		if (solsys->body[i].primary == NULL)
		{
			log_err("Couldn't find %s's primary: %s\n", body->name,
					body->primary_name);
			free(solsys);
			return NULL;
		}
		free(solsys->body[i].primary_name);
		solsys->body[i].primary_name = NULL; /* Won't ever be used again */
		
		body->orbit.epoch = 0;
		body->orbit.period = M_TWO_PI *
				sqrt(CUBE(body->orbit.SMa) / body->primary->grav_param);
		body->orbit.plane_orientation = quat_euler(RAD(body->orbit.LAN),
				RAD(body->orbit.Inc),
				RAD(body->orbit.APe));
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
		log_dbg("Loaded a solarsystem with %d bodies\n", ret->num_bodies);

	al_destroy_config(cfg);
	al_fclose(file);

	return ret;
}

static void update_body_position(Body *body, double t)
{
	Vec3 v; /* Position relative to the primary */
	if (body->primary)
	{
		v = kepler_position_at_time(&body->orbit, t);
		update_body_position(body->primary, t);
		body->position = vec3_add(body->primary->position, v);
	} else
	{
		body->position = (Vec3) {0, 0, 0};
	}
		
	return;
}	

void solsys_update(SolarSystem *solsys, double t)
{
	int i;

	for (i = 0; i < solsys->num_bodies; i++)
		update_body_position(&solsys->body[i], t);
}
		
