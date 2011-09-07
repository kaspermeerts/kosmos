#ifndef _GLM_H_
#define _GLM_H_

#include <GL/gl.h>
#include "mathlib.h"

GLvoid glmLoadIdentity(Matrix *mat);
Matrix *glmNewMatrixStack(GLvoid);
GLvoid glmFreeMatrixStack(Matrix *mat);
GLvoid glmMultMatrix(Matrix *a, GLdouble m[16]);
GLvoid glmScale(Matrix *mat, GLdouble x, GLdouble y, GLdouble z);
GLvoid glmScaleUniform(Matrix *mat, GLdouble r);
GLvoid glmTranslate(Matrix *mat, GLdouble x, GLdouble y, GLdouble z);
GLvoid glmRotate(Matrix *mat, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
GLvoid glmTranspose(Matrix *mat);
GLvoid glmTransformVector(GLdouble m[16], GLdouble a[4], GLdouble b[4]);
GLvoid glmOrtho(Matrix *mat, GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble near, GLdouble far);
GLvoid glmFrustum(Matrix *mat, GLdouble l, GLdouble r,
		GLdouble b, GLdouble t, GLdouble near, GLdouble far);
GLvoid glmPerspective(Matrix *mat, GLdouble fov, GLdouble aspect, GLdouble near,
		GLdouble far);
GLvoid glmLookAt(Matrix *mat, GLdouble eyex, GLdouble eyey, GLdouble eyez, 
		GLdouble centerx, GLdouble centery, GLdouble centerz,
		GLdouble upx, GLdouble upy,	GLdouble upz);
GLvoid glmUniformMatrix(GLint location, Matrix *mat);

#endif
