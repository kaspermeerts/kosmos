#ifndef KOSMOS_ORBIT_H
#define KOSMOS_ORBIT_H

#include "mathlib.h"

typedef struct Orbit {
	double eccentricity;
	double semimajor_axis;
	double inclination;
	double ascending_node;
	double argument_of_periapsis;
	double mean_anomaly;

	double period;
	Mat3 plane_orientation;
} Orbit;

Vec3 position_at_time(Orbit, double);

#endif
