#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include "quaternion.h"
#include "matrix.h"
#include "glm.h"

GLvoid glmUniformMatrix(GLint location, Matrix *mat)
{
	GLfloat m[16];
	int i;

	for (i = 0; i < 16; i++)
		m[i] = (GLfloat) mat->m[i];

	glUniformMatrix4fv(location, 1, GL_FALSE, m);
}

GLvoid glmLoadIdentity(Matrix *mat)
{
	mat_load_identity(mat);
}

Matrix *glmNewMatrixStack(void)
{
	return mat_push(NULL);
}

GLvoid glmFreeMatrixStack(Matrix *mat)
{
	mat_free(mat);
}

/* Multiply m on the right with a scaling matrix */
GLvoid glmScale(Matrix *mat, GLdouble x, GLdouble y, GLdouble z)
{
	GLdouble *m = mat->m;

	m[ 0] *= x; m[ 4] *= y; m[ 8] *= z;
	m[ 1] *= x; m[ 5] *= y; m[ 9] *= z;
	m[ 2] *= x; m[ 6] *= y; m[10] *= z;
	m[ 3] *= x; m[ 7] *= y; m[11] *= z;
}

GLvoid glmScaleUniform(Matrix *mat, GLdouble r)
{
	glmScale(mat, r, r, r);
}

GLvoid glmTranslate(Matrix *mat, GLdouble tx, GLdouble ty, GLdouble tz)
{
	GLdouble *m = mat->m;
#define M(i, j) m[4*j + i]
	M(0, 3) = M(0, 0)*tx + M(0, 1)*ty + M(0, 2)*tz + M(0, 3);
	M(1, 3) = M(1, 0)*tx + M(1, 1)*ty + M(1, 2)*tz + M(1, 3);
	M(2, 3) = M(2, 0)*tx + M(2, 1)*ty + M(2, 2)*tz + M(2, 3);
	M(3, 3) = M(3, 0)*tx + M(3, 1)*ty + M(3, 2)*tz + M(3, 3);
#undef M
}

GLvoid glmRotate(Matrix *mat, GLdouble angle, GLdouble ax,
		GLdouble ay, GLdouble az)
{
	Quaternion q;

	q = quat_from_angle_axis(angle, ax, ay, az);
	mat_mul_quaternion(mat, q);
}

/* Perspective manipulation */
GLvoid glmOrtho(Matrix *mat, GLdouble left, GLdouble right,
		GLdouble bottom, GLdouble top, GLdouble near, GLdouble far)
{
	GLdouble m[16];

	glmLoadIdentity(mat);
#define M(i, j) m[4*j + i]
	M(0, 0) = 2.0 / (right - left);
	M(0, 3) = -(right + left) / (right - left);

	M(1, 1) = 2.0 / (top - bottom);
	M(1, 3) = -(top + bottom) / (top - bottom);

	M(2, 2) = 2.0 / (far - near);
	M(2, 3) = -(far  +  near) / (far  -  near);

#undef M

	mat_mul(mat, m);
}

GLvoid glmFrustum(Matrix *mat, GLdouble l, GLdouble r,
		GLdouble b, GLdouble t, GLdouble near, GLdouble far)
{
	GLdouble x, y, A, B, C, D;

	x = 2*near/(r - l);
	y = 2*near/(t - b);
	A = (r + l)/(r - l);
	B = (t + b)/(t - b);
	C = (far + near)/(far - near);
	D = 2 * far * near / (far - near);

#define M(i, j) mat->m[4*j + i]
   M(0,0) = x;  M(0,1) = 0;  M(0,2) =  A;  M(0,3) = 0;
   M(1,0) = 0;  M(1,1) = y;  M(1,2) =  B;  M(1,3) = 0;
   M(2,0) = 0;  M(2,1) = 0;  M(2,2) =  C;  M(2,3) = D;
   M(3,0) = 0;  M(3,1) = 0;  M(3,2) = -1;  M(3,3) = 0;
#undef  M
}

GLvoid glmPerspective(Matrix *mat, GLdouble fov, GLdouble aspect, GLdouble near,
		GLdouble far)
{
	GLdouble m[16];
	GLdouble cotan = 1/tan(fov);
	GLdouble depth = far - near;

#define M(i, j) m[4*j + i]
	M(0, 0) = cotan;
	M(0, 1) = 0;
	M(0, 2) = 0;
	M(0, 3) = 0;

	M(1, 0) = 0;
	M(1, 1) = aspect*cotan;
	M(1, 2) = 0;
	M(1, 3) = 0;
	
	M(2, 0) = 0;
	M(2, 1) = 0;
	M(2, 2) = -(far + near)/depth;
	M(2, 3) = -2*far*near/depth;
	
	M(3, 0) = 0;
	M(3, 1) = 0;
	M(3, 2) = -1;
	M(3, 3) = 0;
#undef M

	mat_mul(mat, m);
}
