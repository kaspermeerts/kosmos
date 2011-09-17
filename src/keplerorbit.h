#ifndef KOSMOS_KEPLERORBIT_H
#define KOSMOS_KEPLERORBIT_H

#include "mathlib.h"

typedef struct KeplerOrbit {
	double Ecc; /* Eccentricity */
	double SMa; /* Semi-major axis */
	double Inc; /* Inclination */
	double LAN; /* Longitude of ascending node */
	double APe; /* Argument of periapsis */
	double MnA; /* Mean anomaly at epoch */

	double epoch;
	double period;
	Mat3 plane_orientation;
} KeplerOrbit;

Vec3 kepler_position_at_true_anomaly(KeplerOrbit *orbit, double theta);
Vec3 kepler_position_at_E(KeplerOrbit *orbit, double E);
Vec3 kepler_position_at_time(KeplerOrbit *orbit, double jd);
#endif
