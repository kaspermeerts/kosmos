#ifndef KOSMOS_MATRIX_H
#define KOSMOS_MATRIX_H

#include "vector.h"

typedef double Mat3[9];

void mat3_mult(Mat3 a, Mat3 b, Mat3 c);
Vec3 mat3_transform(Mat3 m, Vec3 v);

#endif
