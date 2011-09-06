#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "matrix.h"
#include "vector.h"
#include "quaternion.h"

typedef struct Camera {
	Vec3 position; /* position of the camera itself */
	Vec3 target;   /* position of the current selection*/
	Quaternion orientation; /* orientation of the camera in space */
	
	int x, y; /* the lower left corner of the viewport */
	int width, height;

	double fov; /* lens length ? */
	double zNear;
	double zFar;
} Camera;

void cam_projection_matrix(const Camera *cam, Matrix *mat);
void cam_view_matrix(const Camera *cam, Matrix *mat);
void cam_lookat(Camera *cam, Vec3 position, Vec3 center, Vec3 up);
void cam_orbit(Camera *cam, int dx, int dy);

#endif
