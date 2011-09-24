#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>

#include "mathlib.h"
#include "keplerorbit.h"

static double solve_kepler_equation(double ecc, double M);

static double solve_kepler_equation(double ecc, double M)
{
	double E0, E1, E2, E3;

	E0 = M; /* FIXME */
	E1 = M + ecc*sin(E0);
	E2 = M + ecc*sin(E1);
	E3 = M + ecc*sin(E2);

	return E3;
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
	double M, E, mean_motion; /* true, mean and eccentric anomaly */
	mean_motion = M_TWO_PI*t/orbit->period;
	M = orbit->MnA + mean_motion;
	E = solve_kepler_equation(orbit->Ecc, M);

	return kepler_position_at_E(orbit, E);
}
