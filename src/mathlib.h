#ifndef KOSMOS_MATHLIB_H
#define KOSMOS_MATHLIB_H

#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif
#define M_TWO_PI 6.283185307179586476925287

#ifndef GRAV_CONST
#define GRAV_CONST 6.67384e-11
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(x) ((a) > 0 ? (a) : -(a))
#define SQUARE(x) ((x) * (x))
#define CUBE(x) ((x) * (x) * (x))

#include "quaternion.h"
#include "vector.h"
#include "matrix.h"

#endif
