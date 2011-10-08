#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "mathlib.h"
#include "keplerorbit.h"
#include "log.h"

static double solve_kepler_equation(const double ecc, const double M)
{
	double E, E0;
	int i = 0;
	double TOLERANCE = 1e-9; /* XXX Complete arbitrary */

	E0 = 0;
	E = M;
	if (ecc == 0)
		E = M;
	else if (ecc < 0.2)
	{
		for (i = 0; fabs(E - E0) > TOLERANCE; i++)
		{
			E0 = E;
			E = M + ecc*sin(E);
		}
	} else if (ecc < 0.9999) /* TODO */
	{
		for (i = 0; fabs(E - E0) > TOLERANCE; i++)
		{
			E0 = E;
			E = E + (M + ecc * sin(E) - E) / (1 - ecc * cos(E));
		}
	} else
	{
		log_err("Eccentricity too high %g\n", ecc);
		return 0;
	}

	return E;
}

Vec3 kepler_position_at_true_anomaly(KeplerOrbit *orbit, double theta)
{
	double e = orbit->Ecc;
	double p = orbit->SMa * (1-e*e);
	Vec3 plane_pos;

	plane_pos.x = p / (1 + e*cos(theta)) * cos(theta);
	plane_pos.y = p / (1 + e*cos(theta)) * sin(theta);
	plane_pos.z = 0;

	return quat_transform(orbit->plane_orientation, plane_pos);
}

Vec3 kepler_position_at_E(KeplerOrbit *orbit, double E)
{
	double e = orbit->Ecc;
	double a = orbit->SMa, b = a * sqrt(1 - e*e);
	Vec3 plane_pos;

	plane_pos.x = a * (cos(E) - e);
	plane_pos.y = b * sin(E);
	plane_pos.z = 0;

	return quat_transform(orbit->plane_orientation, plane_pos);
}

Vec3 kepler_position_at_time(KeplerOrbit *orbit, double jd)
{
	double t = jd - orbit->epoch;
	double M, E, mean_motion; /* mean and eccentric anomaly */
	mean_motion = M_TWO_PI*t/orbit->period;
	M = RAD(orbit->MnA) + mean_motion;
	E = solve_kepler_equation(orbit->Ecc, M);

	return kepler_position_at_E(orbit, E);
}
