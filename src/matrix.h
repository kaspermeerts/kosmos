#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "quaternion.h"

typedef struct Matrix {
	double m[16];
	struct Matrix *next;
} Matrix;

void mat_print(const Matrix *mat);
void mat_copy(Matrix *dest, const Matrix *src);
void mat_load_identity(Matrix *mat);
Matrix *mat_new(void);
void mat_free(Matrix *mat);
Matrix *mat_push(Matrix *mat);
Matrix *mat_pop(Matrix *mat);
void mat_load(Matrix *mat, double m[16]);
void mat_mul(Matrix *a, double m[16]);
void mat_transpose(Matrix *mat);
void mat_transform_vector(double m[16], double a[4], double b[4]);
void mat_mul_quaternion(Matrix *mat, Quaternion q);

#endif
