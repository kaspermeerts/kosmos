#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include "glm.h"
#include "mathlib.h"

static void matrix_mul_matrix(double *c, double *a, double *b);
static void matrix_from_quat(double m[16], const Quaternion p);

static void matrix_mul_matrix(double *c, double *a, double *b)
{
#define A(i, j) a[4*j + i]
#define B(i, j) b[4*j + i]
#define C(i, j) c[4*j + i]
	int i, j;

	for (i = 0; i < 4; i++)
	{
		const double Ai0 = A(i,0), Ai1 = A(i,1), Ai2 = A(i,2), Ai3 = A(i,3);
		for (j = 0; j < 4; j++)
			C(i,j) = Ai0*B(0,j) + Ai1*B(1,j) + Ai2*B(2,j) + Ai3*B(3,j);
	}
#undef C
#undef B
#undef A
}

/* The Euler-Rodrigues formula */
static void matrix_from_quat(double m[16], const Quaternion p)
{
	double w = p.w, x = p.x, y = p.y, z = p.z;

#define M(i, j) m[4*j + i]
	M(0, 0) = w*w + x*x - y*y - z*z;
	M(0, 1) = 2*x*y - 2*w*z;
	M(0, 2) = 2*x*z + 2*w*y;
	M(0, 3) = 0.0;

	M(1, 0) = 2*x*y + 2*w*z;
	M(1, 1) = w*w - x*x + y*y - z*z;
	M(1, 2) = 2*y*z - 2*w*x;
	M(1, 3) = 0.0;

	M(2, 0) = 2*x*z - 2*w*y;
	M(2, 1) = 2*y*z + 2*w*x;
	M(2, 2) = w*w - x*x - y*y + z*z;
	M(2, 3) = 0.0;

	M(3, 0) = 0.0;
	M(3, 1) = 0.0;
	M(3, 2) = 0.0;
	M(3, 3) = 1.0;
#undef M
}

GLvoid glmPrintMatrix(Matrix *mat)
{
	const double *m = mat->m;
	printf("%g\t%g\t%g\t%g\n", m[ 0], m[ 4], m[ 8], m[12]);
	printf("%g\t%g\t%g\t%g\n", m[ 1], m[ 5], m[ 9], m[13]);
	printf("%g\t%g\t%g\t%g\n", m[ 2], m[ 6], m[10], m[14]);
	printf("%g\t%g\t%g\t%g\n", m[ 3], m[ 7], m[11], m[15]);
}

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
	memset(mat->m, 0, 16 * sizeof(double));
	mat->m[0] = mat->m[5] = mat->m[10] = mat->m[15] = 1.0;
}

Matrix *glmNewMatrixStack(void)
{
	return glmPushMatrix(NULL);
}

Matrix *glmPopMatrix(Matrix *mat)
{
	Matrix *next;

	next = mat->next;
	free(mat);
	return next;
}

Matrix *glmPushMatrix(Matrix *mat)
{
	Matrix *new;

	new = malloc(sizeof(Matrix));
	if (new == NULL)
		return mat;

	glmLoadIdentity(new);
	new->next = mat;

	return new;
}

GLvoid glmFreeMatrixStack(Matrix *mat)
{
	while(mat != NULL)
		mat = glmPopMatrix(mat);
}

GLvoid glmLoadMatrix(Matrix *mat, GLdouble m[16])
{
	memcpy(mat->m, m, sizeof(double) * 16);
}

GLvoid glmMultMatrix(Matrix *mat, GLdouble m[16])
{
	matrix_mul_matrix(mat->m, mat->m, m);
}

GLvoid glmMultQuaternion(Matrix *mat, Quaternion q)
{
	double m[16];

	matrix_from_quat(m, q);

	glmMultMatrix(mat, m);
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
	glmMultQuaternion(mat, q);
}

/****************************
 * Perspective manipulation *
 ****************************/
GLvoid glmOrtho(Matrix *mat, GLdouble left, GLdouble right,
		GLdouble bottom, GLdouble top, GLdouble near, GLdouble far)
{
	GLdouble m[16];

	memset(m, 0, sizeof(m));

#define M(i, j) m[4*j + i]
	M(0, 0) = 2.0 / (right - left);
	M(0, 3) = -(right + left) / (right - left);

	M(1, 1) = 2.0 / (top - bottom);
	M(1, 3) = -(top + bottom) / (top - bottom);

	M(2, 2) = 2.0 / (far - near);
	M(2, 3) = -(far  +  near) / (far  -  near);

	M(3, 3) = 1.0;
#undef M

	glmMultMatrix(mat, m);
}

GLvoid glmFrustum(Matrix *mat, GLdouble l, GLdouble r,
		GLdouble b, GLdouble t, GLdouble near, GLdouble far)
{
	GLdouble x, y, A, B, C, D, m[16];

	x = 2*near/(r - l);
	y = 2*near/(t - b);
	A = (r + l)/(r - l);
	B = (t + b)/(t - b);
	C = (far + near)/(far - near);
	D = 2 * far * near / (far - near);

#define M(i, j) m[4*j + i]
   M(0,0) = x;  M(0,1) = 0;  M(0,2) =  A;  M(0,3) = 0;
   M(1,0) = 0;  M(1,1) = y;  M(1,2) =  B;  M(1,3) = 0;
   M(2,0) = 0;  M(2,1) = 0;  M(2,2) =  C;  M(2,3) = D;
   M(3,0) = 0;  M(3,1) = 0;  M(3,2) = -1;  M(3,3) = 0;
#undef  M

	glmMultMatrix(mat, m);
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

	glmMultMatrix(mat, m);
}
