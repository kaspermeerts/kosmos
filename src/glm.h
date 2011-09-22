#ifndef KOSMOS_GLM_H
#define KOSMOS_GLM_H

#include <GL/gl.h>
#include "mathlib.h"

typedef struct Matrix {
	double m[16];
	struct Matrix *next;
} Matrix;

GLvoid glmPrintMatrix(Matrix *mat);
GLvoid glmUniformMatrix(GLint location, Matrix *mat);
GLvoid glmLoadIdentity(Matrix *mat);
Matrix *glmNewMatrixStack(void);
GLvoid glmPopMatrix(Matrix **mat);
GLvoid glmPushMatrix(Matrix **mat);
GLvoid glmFreeMatrixStack(Matrix *mat);
GLvoid glmLoadMatrix(Matrix *mat, GLdouble m[16]);
GLvoid glmMultMatrix(Matrix *mat, GLdouble m[16]);
GLvoid glmMultQuaternion(Matrix *mat, Quaternion q);
Vec3 glmTransformVector(Matrix *mat, Vec3);
GLvoid glmScale(Matrix *mat, GLdouble x, GLdouble y, GLdouble z);
GLvoid glmScaleUniform(Matrix *mat, GLdouble r);
GLvoid glmTranslate(Matrix *mat, GLdouble tx, GLdouble ty, GLdouble tz);
GLvoid glmRotate(Matrix *mat, GLdouble angle, GLdouble ax,
		GLdouble ay, GLdouble az);
GLvoid glmOrtho(Matrix *mat, GLdouble left, GLdouble right,
		GLdouble bottom, GLdouble top, GLdouble near, GLdouble far);
GLvoid glmFrustum(Matrix *mat, GLdouble l, GLdouble r,
		GLdouble b, GLdouble t, GLdouble near, GLdouble far);
GLvoid glmPerspective(Matrix *mat, GLdouble fov, GLdouble aspect, GLdouble near,
		GLdouble far);
#endif
