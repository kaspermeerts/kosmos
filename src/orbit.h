#ifndef _ORBIT_H_
#define _ORBIT_H_

#include "vecmath.h"

typedef struct Orbit {
	double eccentricity;
	double semimajor_axis;
	double inclination;
	double ascending_node;
	double argument_of_periapsis;
	double mean_anomaly;

	double period;

	Mat3 orientation_matrix;
} Orbit;

Vec3 position_at_time(Orbit, double);

#endif
