#ifndef KOSMOS_SHADER_H
#define KOSMOS_SHADER_H

#include <GL/gl.h>

typedef struct {
	GLuint program;
	GLuint vertex_shader;
	GLuint fragment_shader;

	GLint location[12];
} Shader;

#define SHADER_ATT_POSITION 0
#define SHADER_ATT_NORMAL   1
#define SHADER_ATT_TEXCOORD 2
#define SHADER_UNI_M_MATRIX 3
#define SHADER_UNI_V_MATRIX 4
#define SHADER_UNI_P_MATRIX 5
#define SHADER_UNI_LIGHT_POS 6
#define SHADER_UNI_LIGHT_AMBIENT 7
#define SHADER_UNI_LIGHT_DIFFUSE 8
#define SHADER_UNI_LIGHT_SPECULAR 9

Shader *shader_create(const char *vertex_source, const char *fragment_source);
void shader_delete(Shader *shader);
#endif
