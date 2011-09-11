#ifndef _SHADER_H_
#define _SHADER_H_

#include <GL/gl.h>

typedef struct {
	GLuint program;
	GLuint vertex_shader;
	GLuint fragment_shader;

	GLint attribute_vertex;
	GLint attribute_normal;
	GLint uniform_mv_matrix;
	GLint uniform_p_matrix;
	GLint uniform_light_dir;
	GLint uniform_mat_ambient;
	GLint uniform_mat_diffuse;
	GLint uniform_mat_specular;
	GLint uniform_mat_shininess;
} Shader;

Shader *shader_create(const char *vertex_source, const char *fragment_source);
void shader_delete(Shader *shader);
#endif 
