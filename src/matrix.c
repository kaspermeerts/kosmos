#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mathlib.h"
#include "matrix.h"

static void matrix_mul_matrix(double *prod, double *a, double *b);
static void matrix_from_quat(double m[16], const Quaternion p);

void mat_print(const Matrix *mat)
{
	const double *m = mat->m;
	printf("%+5.3f\t%+5.3f\t%+5.3f\t%+5.3f\n", m[ 0], m[ 4], m[ 8], m[12]);
	printf("%+5.3f\t%+5.3f\t%+5.3f\t%+5.3f\n", m[ 1], m[ 5], m[ 9], m[13]);
	printf("%+5.3f\t%+5.3f\t%+5.3f\t%+5.3f\n", m[ 2], m[ 6], m[10], m[14]);
	printf("%+5.3f\t%+5.3f\t%+5.3f\t%+5.3f\n", m[ 3], m[ 7], m[11], m[15]);
}

void mat_copy(Matrix *dest, const Matrix *src)
{
	memcpy(dest->m, src->m, sizeof(double) * 16);
}

void mat_load_identity(Matrix *mat)
{
	memset(mat->m, 0, 16 * sizeof(double));
	mat->m[0] = mat->m[5] = mat->m[10] = mat->m[15] = 1.0;
}

Matrix *mat_new(void)
{
	return mat_push(NULL);
}

void mat_free(Matrix *mat)
{
	while((mat = mat_pop(mat)) != NULL)
		;
}

Matrix *mat_push(Matrix *mat)
{
	Matrix *new;

	new = malloc(sizeof(Matrix));
	if (new == NULL)
		return mat; /* FIXME */

	mat_load_identity(new);
	new->next = mat;

	return new;
}

Matrix *mat_pop(Matrix *mat)
{
	Matrix *next;

	next = mat->next;
	free(mat);
	return next;
}

void mat_load(Matrix *mat, double m[16])
{
	memcpy(mat->m, m, sizeof(double) * 16);
}

void mat_mul(Matrix *a, double m[16])
{
	matrix_mul_matrix(a->m, a->m, m);
}

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

/* Transpose a matrix in-place */
void mat_transpose(Matrix *mat)
{
	double *m = mat->m;

	double temp;
#define SWAP(a, b) temp = a; a = b; b = temp;
	SWAP(m[ 1], m[ 4]);
	SWAP(m[ 2], m[ 8]);
	SWAP(m[ 3], m[12]);
	SWAP(m[ 6], m[ 9]);
	SWAP(m[ 7], m[13]);
	SWAP(m[11], m[14]);
#undef SWAP
}

#if 0
Vec4 TransformVector(Matrix *mat, double a[4], double b[4])
{
	const double a0 = a[0], a1 = a[1], a2 = a[2], a3 = a[3];

#define M(x,y) m[x*4 + y]
	b[0] = M(0,0)*a0 + M(0,1)*a1 + M(0,2)*a2 + M(0,3)*a3;
	b[1] = M(1,0)*a0 + M(1,1)*a1 + M(1,2)*a2 + M(1,3)*a3;
	b[2] = M(2,0)*a0 + M(2,1)*a1 + M(2,2)*a2 + M(2,3)*a3;
	b[3] = M(3,0)*a0 + M(3,1)*a1 + M(3,2)*a2 + M(3,3)*a3;
#undef M
}
#endif

/* Quaternion manipulation */
void mat_mul_quaternion(Matrix *mat, Quaternion p)
{
	double m[16];

	matrix_from_quat(m, p);

	matrix_mul_matrix(mat->m, mat->m, m);
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
