#include <stdio.h>
#include <math.h>

#include "camera.h"
#include "quaternion.h"
#include "matrix.h"
#include "vector.h"
#include "glm.h"

void cam_lookat(Camera *cam, Vec3 pos, Vec3 target, Vec3 up)
{
	Vec3 forward, side;

	/* Create an orthonormal basis */
	forward = vec3_normalize(vec3_sub(target, pos));
	side    = vec3_normalize(vec3_cross(forward, up));
	up      = vec3_normalize(vec3_cross(side, forward)); /* FIXME: Already normalized */

	/* Create a quaternion out of these */
	/* FIXME: Numerically unstable */
	Quaternion q;
	double trace, w4r;
	
	trace = side.x + up.y - forward.z;
	if (trace < -1 + 1e-3)
		printf("TRACE UNDERFLOW\n");
	q.w = 0.5*sqrt(1 + trace);
	w4r = 1 / ( 4*q.w);
	q.x = -((-forward.y) - up.z     ) * w4r;
	q.y = -( side.z    - (-forward.x)) * w4r;
	q.z = -( up.x      - side.y   ) * w4r;

	cam->target = target;
	cam->position = pos;
	cam->orientation = q;
}

void cam_projection_matrix(const Camera *cam, Matrix *proj)
{
	glmPerspective(proj, cam->fov, cam->aspect, cam->zNear, cam->zFar);
}

void cam_view_matrix(const Camera *cam, Matrix *view)
{
	mat_load_identity(view);
	/* Because the matrix transform vertices, we have to give
	 * the inverse transformation */
	mat_mul_quaternion(view, quat_conjugate(cam->orientation));
	glmTranslate(view, -cam->position.x, -cam->position.y, -cam->position.z);

}

void cam_orbit(Camera *cam, int dx, int dy)
{
	/* FIXME: Cleanup */
	/* FIXME: Drift */
	Quaternion q, q2;
	Quaternion o = cam->orientation;
	Vec3 v;

	/* Conjugate because camera is inverse FIXME: derp */
	q = quat_conjugate(quat_trackball(dx, dy));

	/* because of premultiplication ? FIXME */
	q2 = quat_multiply(o, quat_multiply(q, quat_conjugate(o)));
	q2 = quat_normalize(q2);

	v = vec3_sub(cam->position, cam->target);
	v = quat_transform_vector(q2, v);
	cam->position = vec3_add(cam->target, v);

	cam->orientation = quat_multiply(cam->orientation, q);
}
