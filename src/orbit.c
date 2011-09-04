#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "vecmath.h"
#include "orbit.h"

#ifndef TWO_PI
#define TWO_PI 6.2831 /* XXX */
#endif

static double solve_kepler_equation(double ecc, double M)
{
	double E0, E1, E2, E3;

	E0 = M;
	E1 = M + ecc*sin(E0);
	E2 = M + ecc*sin(E1);
	E3 = M + ecc*sin(E2);

	return E3;
}

static Vec3 position_at_E(Orbit orbit, double E)
{
	double e = orbit.eccentricity;
	double a = orbit.semimajor_axis, b = a * sqrt(1 - e*e);
	Vec3 plane_pos;

	plane_pos.x = a * (cos(E) - e);
	plane_pos.y = b * sin(E);
	plane_pos.z = 0;

	return transform_vector(orbit.orientation_matrix, plane_pos);
}

Vec3 position_at_time(Orbit orbit, double jd)
{
	double t = jd;
	double M, E, mean_motion; /* true, mean and eccentric anomaly */
	mean_motion = TWO_PI*t/orbit.period;
	M = orbit.mean_anomaly + mean_motion;
	E = solve_kepler_equation(orbit.eccentricity, M);

	return position_at_E(orbit, E);
}

int main(int argc, char **argv)
{
	double ecc, M;
	if (argc < 3)
		return 1;

	ecc = atof(argv[1]);
	M = atof(argv[2]);
	printf("%f\n", solve_kepler_equation(ecc, M));

	return 0;
}
