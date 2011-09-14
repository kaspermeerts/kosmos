#ifndef _MATHLIB_H_
#define _MATHLIB_H_

#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif
#define M_TWO_PI 6.283185307179586476925287

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(x) ((a) > 0 ? (a) : -(a))
#define SQUARE(x) ((x) * (x))

#include "quaternion.h"
#include "vector.h"

#endif
