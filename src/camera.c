#include "camera.h"
#include "matrix.h"
#include "vector.h"
#include "glm.h"

void cam_lookat(Camera *cam, Vec3 pos, Vec3 center, Vec3 up)
{
	Vec3 forward, side;

	/* Create an orthonormal basis, this will determine the rotation matrix */
	forward = vec3_sub(center, pos);
	side = vec3_cross(forward, up);
	up = vec3_cross(side, forward);

	cam->position = pos;
	cam->forward = vec3_normalize(forward);
	cam->side = vec3_normalize(side);
	cam->up = vec3_normalize(up);
}

void cam_projection_matrix(const Camera *cam, Matrix *proj)
{
	glmPerspective(proj, cam->fov, cam->aspect, cam->zNear, cam->zFar);
}

void cam_view_matrix(const Camera *cam, Matrix *view)
{
	GLdouble m[16] = {0};

#define M(i, j)   m[4*i + j]
	M(0, 0) = cam->side.x;
	M(1, 0) = cam->side.y;
	M(2, 0) = cam->side.z;
	M(3, 0) = 0.0;

	M(0, 1) = cam->up.x;
	M(1, 1) = cam->up.y;
	M(2, 1) = cam->up.z;
	M(3, 1) = 0.0;

	M(0, 2) = -cam->forward.x;
	M(1, 2) = -cam->forward.y;
	M(2, 2) = -cam->forward.z;
	M(3, 2) = 0.0;

	M(0, 3) = 0.0; 
	M(1, 3) = 0.0;
	M(2, 3) = 0.0;
	M(3, 3) = 1.0;
#undef M

	mat_load_identity(view);
	mat_mul(view, m);
	glmTranslate(view, -cam->position.x, -cam->position.y, -cam->position.z);
}
