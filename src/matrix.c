#include "matrix.h"


void mat3_mult(Mat3 a, Mat3 b, Mat3 c)
{
	int i, j;

#define A(i,j) a[3*j + i]
#define B(i,j) b[3*j + i]
#define C(i,j) c[3*j + i]
	for (i = 0; i < 3; i++)
	{
		double A0 = A(i, 0), A1 = A(i, 1), A2 = A(i, 2);
		for (j = 0; j < 3; j++)
			C(i,j) = A0*B(0,j) + A1*B(1,j) + A2*B(2,j);
	}
#undef C
#undef B
#undef A

}

Vec3 mat3_transform(Mat3 a, Vec3 v)
{
	Vec3 w;

#define A(i, j) a[3*j + i]
	w.x = A(0,0)*v.x + A(0,1)*v.y + A(0,2)*v.z;
	w.y = A(1,0)*v.x + A(1,1)*v.y + A(1,2)*v.z;
	w.z = A(2,0)*v.x + A(2,1)*v.y + A(2,2)*v.z;
#undef A
	
	return w;
}
