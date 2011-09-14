#ifndef KOSMOS_CAMERA_H
#define KOSMOS_CAMERA_H

#include "glm.h"
#include "vector.h"
#include "quaternion.h"

typedef struct Camera {
	Vec3 position; /* position of the camera itself */
	Quaternion orientation; /* orientation of the camera in space */

	Vec3 target;   /* position of the current selection*/

	int left, bottom; /* the lower left corner of the viewport */
	int width, height;

	double fov; /* lens length ? */
	double zNear;
	double zFar;
} Camera;

void cam_projection_matrix(const Camera *cam, Matrix *mat);
void cam_view_matrix(const Camera *cam, Matrix *mat);
void cam_lookat(Camera *cam, Vec3 position, Vec3 center, Vec3 up);
void cam_rotate(Camera *cam, int dx, int dy);
void cam_orbit(Camera *cam, int dx, int dy);
void cam_dolly(Camera *cam, int dz);

#endif
