#ifndef KOSMOS_GLM_H
#define KOSMOS_GLM_H

#include <GL/gl.h>
#include "mathlib.h"

typedef struct Vertex3 {
	GLfloat x, y, z;
} Vertex3;

typedef struct Vertex3N {
	GLfloat x, y, z;
	GLfloat nx, ny, nz;
} Vertex3N;

typedef struct Vertex3NT {
	GLfloat x, y, z;
	GLfloat nx, ny, nz;
	GLfloat u, v;
} Vertex3NT;

typedef struct Vertex3T {
	GLfloat x, y, z;
	GLfloat u, v;
} Vertex3T;

typedef struct Vertex2 {
	GLfloat x, y;
} Vertex2;

typedef struct Vertex2C {
	GLfloat x, y;
	GLclampf r, g, b, a;
} Vertex2C;

/* Colour and texture? The texture here is a luminance texture, with no colour
 * information. This is e.g. used for text */
typedef struct Vertex2CT {
	GLfloat x, y;
	GLfloat u, v;
	GLclampf r, g, b, a;
} Vertex2CT;

typedef struct Vertex2T {
	GLfloat x, y;
	GLfloat u, v;
} Vertex2T;

typedef struct Matrix Matrix;

extern Matrix *glmProjectionMatrix;
extern Matrix *glmViewMatrix;
extern Matrix *glmModelMatrix;

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
GLvoid glmTranslateVector(Matrix *mat, Vec3 v);
GLvoid glmRotate(Matrix *mat, GLdouble angle, GLdouble ax,
		GLdouble ay, GLdouble az);
GLvoid glmOrtho(Matrix *mat, GLdouble left, GLdouble right,
		GLdouble bottom, GLdouble top, GLdouble near, GLdouble far);
GLvoid glmFrustum(Matrix *mat, GLdouble l, GLdouble r,
		GLdouble b, GLdouble t, GLdouble near, GLdouble far);
GLvoid glmPerspective(Matrix *mat, GLdouble fov, GLdouble aspect, GLdouble near,
		GLdouble far);
#endif
