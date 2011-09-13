#include <stdio.h>
#include <math.h>

#include "camera.h"
#include "mathlib.h"
#include "glm.h"

void cam_lookat(Camera *cam, Vec3 pos, Vec3 target, Vec3 up)
{
	Vec3 forward, side;

	/* Create an orthonormal basis */
	forward = vec3_normalize(vec3_sub(target, pos));
	side    = vec3_normalize(vec3_cross(forward, up));
	up      = vec3_cross(side, forward); /* Already normalized */

	/* Create a quaternion out of these */
	Quaternion q;
	double T;
	
	/* To understand this, research "convert orthogonal matrix to quaternion"
	 * This algorithm comes from "The Matrix and Quaternion FAQ" */
	forward = vec3_scale(forward, -1); /* We are looking down on the z-axis */
	T = 1 + side.x + up.y + forward.z;
	if (T > 1e-3)
	{
		q.w = 0.5*sqrt(T);
		q.x = (up.z - forward.y) / (4*q.w);
		q.y = (forward.x - side.z) / (4*q.w);
		q.z = (side.y - up.x) / (4*q.w);
	} else 
	{
		if (side.x > up.y && side.x > forward.z)
		{
			T = sqrt(1 + side.x - up.y - forward.z);
			q.w = (up.z - forward.y) / (2*T);
			q.x = 0.5*T;
			q.y = (up.x + side.y) / (2*T);
			q.z = (forward.x + side.z) / (2*T);
		} else if (up.y > forward.z)
		{
			T = sqrt(1 - side.x + up.y - forward.z);
			q.w = (forward.x - side.z) / (2*T);
			q.x = (up.x + side.y) / (2*T);
			q.y = 0.5*T;
			q.z = (forward.y + up.z) / (2*T);
		} else
		{
			T = sqrt(1 - side.x - up.y + forward.z);
			q.w = (side.y - up.x) / (2*T);
			q.x = (forward.x + side.z) / (2*T);
			q.y = (forward.y + up.z) / (2*T);
			q.z = 0.5*T;
		}
	}

	cam->target = target;
	cam->position = pos;
	cam->orientation = q;
}

void cam_projection_matrix(const Camera *cam, Matrix *proj)
{
	float aspect = (float) cam->width / (float)cam->height;
	glmPerspective(proj, cam->fov, aspect, cam->zNear, cam->zFar);
}

void cam_view_matrix(const Camera *cam, Matrix *view)
{
	mat_load_identity(view);
	/* Because the matrix transform vertices, we have to give
	 * the inverse transformation */
	mat_mul_quaternion(view, quat_conjugate(cam->orientation));
	glmTranslate(view, -cam->position.x, -cam->position.y, -cam->position.z);

}

void cam_rotate(Camera *cam, int dx, int dy)
{

	const double radius = MIN(cam->width, cam->height)/2;
	Quaternion q;

	q = quat_trackball(dx, dy, radius);
	cam->orientation = quat_multiply(cam->orientation, q);
}

void cam_orbit(Camera *cam, int dx, int dy)
{
	const double radius = MIN(cam->width, cam->height)/2;
	double distance;
	Vec3 v = vec3_sub(cam->position, cam->target);
	Quaternion o = cam->orientation;
	Quaternion q, q2;

	/* We invert the transformation because we are transforming the camera
	 * and not the scene. */
	q = quat_conjugate(quat_trackball(dx, dy, radius));

	/* The quaternion q gives us an intrinsic transformation, close to unity.
	 * To make it extrinsic, we compute q2 = o * q * ~o */
	q2 = quat_multiply(o, quat_multiply(q, quat_conjugate(o)));
	q2 = quat_normalize(q2);

	/* As round-off errors accumulate, the distance between the camera and the
	 * target would normally fluctuate. We take steps to prevent that here. */
	distance = vec3_length(v);
	v = quat_transform(q2, v);
	v = vec3_normalize(v);
	v = vec3_scale(v, distance);

	cam->position = vec3_add(cam->target, v);
	cam->orientation = quat_multiply(q2, cam->orientation);
}

void cam_dolly(Camera *cam, int dz)
{
	Vec3 v;

	v = vec3_sub(cam->position, cam->target);
	v = vec3_scale(v, exp(-0.1*dz));
	cam->position = vec3_add(cam->target, v);
}
