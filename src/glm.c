#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include "glm.h"
#include "mathlib.h"

static void matrix_mul_matrix(double *c, double *a, double *b);
static void mat4_from_mat3(double a[16], Mat3 m);

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

static void mat4_from_mat3(double a[16], Mat3 b)
{
#define A(i, j) a[4*j + i]
#define B(i, j) b[3*j + i]
	A(0,0) = B(0,0); A(0,1) = B(0,1); A(0,2) = B(0,2); A(0,3) = 0.0;
	A(1,0) = B(1,0); A(1,1) = B(1,1); A(1,2) = B(1,2); A(1,3) = 0.0;
	A(2,0) = B(2,0); A(2,1) = B(2,1); A(2,2) = B(2,2); A(2,3) = 0.0;
	A(3,0) = 0.0;    A(3,1) = 0.0;    A(3,2) = 0.0;    A(3,3) = 1.0;
#undef B
#undef A
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
	Matrix *new = NULL;
	glmPushMatrix(&new);
	glmLoadIdentity(new);
	return new;
}

GLvoid glmPopMatrix(Matrix **mat)
{
	Matrix *next;

	next = (*mat)->next;
	free(*mat);
	*mat = next;
}

GLvoid glmPushMatrix(Matrix **mat)
{
	Matrix *new;

	new = malloc(sizeof(Matrix));
	if (new == NULL)
		return;

	if (*mat != NULL)
		memcpy(new->m, (*mat)->m, sizeof(double) * 16);

	new->next = *mat;
	*mat = new;
}

GLvoid glmFreeMatrixStack(Matrix *mat)
{
	while(mat != NULL)
		glmPopMatrix(&mat);
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
	Mat3 m3;
	double m4[16];
	
	mat3_from_quat(m3, q);
	mat4_from_mat3(m4, m3);

	glmMultMatrix(mat, m4);
}

/* Homogeneous transform */
Vec3 glmTransformVector(Matrix *mat, Vec3 v)
{
	const GLdouble v0 = v.x, v1 = v.y, v2 = v.z, v3 = 1;
	Vec3 out;
	
#define M(i,j) mat->m[4*j+i]
	out.x = M(0,0)*v0 + M(0,1)*v1 + M(0,2)*v2 + M(0,3)*v3;
	out.y = M(1,0)*v0 + M(1,1)*v1 + M(1,2)*v2 + M(1,3)*v3;
	out.z = M(2,0)*v0 + M(2,1)*v1 + M(2,2)*v2 + M(2,3)*v3;
//	v.w = M(3,0)*v0 + M(3,1)*v1 + M(3,2)*v2 + M(3,3)*v3;
#undef M
	
	return out;	
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

GLvoid glmTranslateVector(Matrix *mat, Vec3 v)
{
	glmTranslate(mat, v.x, v.y, v.z);
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
