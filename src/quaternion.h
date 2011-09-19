#ifndef KOSMOS_QUATERNION_H
#define KOSMOS_QUATERNION_H

#include "matrix.h"
#include "vector.h"

typedef struct Quaternion {
	double w, x, y, z;
} Quaternion;

double quat_length2(Quaternion p);
double quat_length(Quaternion p);
Quaternion quat_scale(Quaternion p, double scale);
Quaternion quat_normalize(Quaternion p);
Quaternion quat_conjugate(Quaternion p);
Quaternion quat_multiply(Quaternion p, Quaternion q);
Quaternion quat_from_angle_axis(double angle, double ax,
		double ay, double az);
Quaternion quat_trackball(int dx, int dy, double radius);
Quaternion quat_from_mat3(Mat3 m);
void mat3_from_quat(Mat3 m, Quaternion p);
Vec3 quat_transform(Quaternion q, Vec3 v);

#endif
