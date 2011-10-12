#ifndef KOSMOS_SHADER_H
#define KOSMOS_SHADER_H

#include <GL/gl.h>

typedef struct {
	GLuint program;
	GLuint vertex_shader;
	GLuint fragment_shader;

	GLint location[7];
} Shader;

#define SHADER_ATT_POSITION 0
#define SHADER_ATT_COLOUR   1
#define SHADER_ATT_NORMAL   2
#define SHADER_ATT_TEXCOORD 3
#define SHADER_UNI_M_MATRIX 4
#define SHADER_UNI_V_MATRIX 5
#define SHADER_UNI_P_MATRIX 6


Shader *shader_create(const char *vertex_source, const char *fragment_source);
void shader_delete(Shader *shader);
#endif
