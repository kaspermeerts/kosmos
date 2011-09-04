#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "matrix.h"
#include "vector.h"

typedef struct Camera Camera;
struct Camera {
	Vec3 position;

	Vec3 forward;
	Vec3 side;
	Vec3 up;
	
	double aspect;
	double fov;
	double zNear;
	double zFar;
};

void cam_projection_matrix(const Camera *cam, Matrix *mat);
void cam_view_matrix(const Camera *cam, Matrix *mat);
void cam_lookat(Camera *cam, Vec3 position, Vec3 center, Vec3 up);

#endif
