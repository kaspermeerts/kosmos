#include <math.h>

#include "quaternion.h"
#include "vector.h"

/* Square of the norm of the quaternion */
double quat_length2(Quaternion p)
{
	return p.w*p.w + p.x*p.x + p.y*p.y + p.z*p.z;
}

double quat_length(Quaternion p)
{
	return sqrt(quat_length2(p));
}

Quaternion quat_scale(Quaternion p, double scale)
{
	Quaternion q;

	q.w = scale * p.w;
	q.x = scale * p.x;
	q.y = scale * p.y;
	q.z = scale * p.z;

	return q;
}

Quaternion quat_normalize(Quaternion p)
{
	return quat_scale(p, 1/quat_length(p));
}

Quaternion quat_conjugate(Quaternion p)
{
	Quaternion q;

	q.w =  p.w;
	q.x = -p.x;
	q.y = -p.y;
	q.z = -p.z;

	return q;
}

/* Multiply two quaternions: r = p * q */
Quaternion quat_multiply(Quaternion p, Quaternion q)
{
	Quaternion r;
	double a = p.w, b = p.x, c = p.y, d = p.z;
	double w = q.w, x = q.x, y = q.y, z = q.z;

	r.w = a*w - b*x - c*y - d*z;
	r.x = b*w + a*x + c*z - d*y;
	r.y = c*w + a*y + d*x - b*z;
	r.z = d*w + a*z + b*y - c*x;

	return r;
}

Quaternion quat_from_angle_axis(double angle, double ax,
		double ay, double az)
{
	Quaternion q;

	double l2 = ax*ax + ay*ay + az*az;
	double l = sqrt(l2), s = sin(angle/2), c = cos(angle/2);

	/* If the axis is invalid, return the identity */
	if (l2 == 0.0)
	{
		return (Quaternion) {1, 0, 0, 0};
	}

	q.w = c;
	q.x = s*ax/l;
	q.y = s*ay/l;
	q.z = s*az/l;
	
	return q;
}

Quaternion quat_trackball(int dx, int dy, double radius)
{
	double dr, sina, cosa, sina2, cosa2;
	Quaternion q = {1, 0, 0, 0};

	if (dx == 0 && dy == 0)
		return q;

	dr = sqrt(dx*dx + dy*dy);

	sina = dr/radius;
	if (sina >= 1)
		sina = 0;
	cosa = sqrt(1 - sina*sina);

	cosa2 = sqrt((1 + cosa)/2);
	sina2 = sina/(2*cosa2);

	q.w = cosa2;
	q.x = -dy/dr * sina2;
	q.y =  dx/dr * sina2;
	q.z = 0;

	return q;
}

Vec3 quat_transform_vector(Quaternion q, Vec3 v)
{
	Vec3 v2;
	Quaternion p = {0, v.x, v.y, v.z};
	Quaternion q2 = quat_conjugate(q);
	Quaternion p2;

	p2 = quat_multiply(q, p);
	p2 = quat_multiply(p2, q2);

	v2.x = p2.x;
	v2.y = p2.y;
	v2.z = p2.z;

	return v2;
}
