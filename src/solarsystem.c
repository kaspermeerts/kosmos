#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <ralloc.h>

#include "log.h"
#include "solarsystem.h"

static const char *get_next_config_section(ALLEGRO_CONFIG *cfg,
		ALLEGRO_CONFIG_SECTION **section)
{
	if (*section == NULL)
		return al_get_first_config_section(cfg, section);
	else
		return al_get_next_config_section(section);
}

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

	*ret = ralloc_strdup(NULL, value);
	if (*ret == NULL)
	{
		log_err("Out of memory\n");
		return false;
	}

	return true;
}

static bool load_body(ALLEGRO_CONFIG *cfg, const char *fullname,
		Body *body, char **primary_name)
{
	const char *name;
	char *type;

	/* Just checking */
	if (al_get_first_config_entry(cfg, fullname, NULL) == NULL)
	{
		log_err("Section %s not in file\n", fullname);
		return false;
	}

	/* Fill in some common parameters */
	if (!config_get_double(cfg, fullname, "Mass", &body->mass, false))
		return false;

	if (!config_get_double(cfg, fullname, "Gravitational parameter",
			&body->grav_param, true))
	{
		body->grav_param = GRAV_CONST * body->mass;
	}

	if (!config_get_double(cfg, fullname, "Radius", &body->radius, false))
		return false;

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
		ralloc_free(type);
		return false;
	}
	ralloc_free(type);

	/* Does it have a primary or not? 
	 * Full names are of the form of "Primary/Name"
	 * We search backwards to allow for things like "Sol/Earth/Moon" */
	if ((name = strrchr(fullname, '/')) == NULL)
	{
		/* This is a body without a primary */
		body->name = ralloc_strdup(body->ctx, fullname);
		body->type = (body->type == BODY_UNKNOWN ? BODY_STAR : body->type);
		body->primary = NULL;
		*primary_name = NULL;
	} else if (name == fullname) /* No primary name, eg: sec = "/Earth" */
	{
		log_err("Malformed name: %s", fullname);
		return false;
	} else
	{
		const char *c;
		for (c = name - 1; c >= fullname && *c != '/'; c--);
		c++;

		body->name = ralloc_strdup(body->ctx, name + 1);
		body->type = (body->type == BODY_UNKNOWN ? BODY_PLANET : body->type);
		body->primary = NULL; /* Fill in later */
		*primary_name = ralloc_strndup(body->ctx, c, name - c);
	}

	body->num_satellites = 0;
	body->satellite = NULL;

	/* Bodies without primaries can't orbit another body */
	if (*primary_name == NULL)
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
	char **primary_names;
	const char *name;
	long num_bodies;
	ALLEGRO_CONFIG_SECTION *sec;
	int i;

	/* First pass: Determine the number of bodies in the file */
	num_bodies = 0;
	sec = NULL;
	while ((name = get_next_config_section(cfg, &sec)) != NULL)
	{
		if (name[0] != '\0')
			num_bodies++;
	}

	if (num_bodies == 0)
		return NULL; /* Empty solarsystem */

	solsys = ralloc_size(NULL, sizeof(SolarSystem) + num_bodies*sizeof(Body));
	if (solsys == NULL)
		return NULL;
	solsys->num_bodies = num_bodies;
	primary_names = ralloc_array(solsys, char *, num_bodies);
	if (primary_names == NULL)
	{
		ralloc_free(solsys);
		return NULL;
	}

	/* Second pass: Load all celestial bodies */
	i = 0;
	sec = NULL;
	while ((name = get_next_config_section(cfg, &sec)) != NULL && i < num_bodies)
	{
		if (name[0] == '\0')
			continue;

		solsys->body[i].ctx = solsys;
		if (!load_body(cfg, name, &solsys->body[i], &primary_names[i]))
		{
			log_err("Couldn't load body %s\n", name);
			ralloc_free(solsys);
			return NULL;
		}
		i++;
	}

	if (i < num_bodies)
		log_err("Internal consistency error\n");

	/* Third pass: Connect each satellite body to its primary */
	for (i = 0; i < num_bodies; i++)
	{
		Body *body = &solsys->body[i];
		char *primary_name = primary_names[i];
		if (primary_name == NULL) /* Independent body */
			continue;

		/* Look for the primary */
		body->primary = NULL;
		for (int j = 0; j < num_bodies; j++)
		{
			Body *body2 = &solsys->body[j];
			if (strcmp(primary_name, body2->name) == 0)
			{
				body->primary = body2;
				break;
			}
		}
		if (body->primary == NULL)
		{
			log_err("Couldn't find %s's primary: %s\n", body->name,
					primary_name);
			ralloc_free(solsys);
			return NULL;
		}
		ralloc_free(primary_name);
		primary_name = NULL; /* Won't ever be used again */

		body->primary->num_satellites++;
		body->primary->satellite = reralloc(solsys, body->primary->satellite,
				Body *, body->primary->num_satellites);
		if (body->primary->satellite == NULL)
		{
			log_err("Out of memory\n");
			ralloc_free(solsys);
			return NULL;
		}
		body->primary->satellite[body->primary->num_satellites - 1] = body;

		body->orbit.epoch = 0;
		body->orbit.period = M_TWO_PI *
				sqrt(CUBE(body->orbit.SMa) / body->primary->grav_param);
		body->orbit.plane_orientation = quat_euler(RAD(body->orbit.LAN),
				RAD(body->orbit.Inc),
				RAD(body->orbit.APe));
	}

	ralloc_free(primary_names);
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

static void update_satellites(Body *body, double t)
{
	for (int i = 0; i < body->num_satellites; i++)
	{
		Vec3 v = kepler_position_at_time(&body->satellite[i]->orbit, t);
		body->satellite[i]->position = vec3_add(body->position, v);
		update_satellites(body->satellite[i], t);
	}

	return;
}

void solsys_update(SolarSystem *solsys, double t)
{
	int i;

	for (i = 0; i < solsys->num_bodies; i++)
	{
		if (solsys->body[i].primary == NULL)
		{
			solsys->body[i].position = (Vec3) {0, 0, 0};
			update_satellites(&solsys->body[i], t);
		}
	}
}

