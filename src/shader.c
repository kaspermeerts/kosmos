#include <stdlib.h>
#include <stdio.h>
#include <allegro5/allegro.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include "shader.h"

static GLuint shader_load(const char *file, GLenum type);
static void show_info_log(GLuint object, PFNGLGETSHADERIVPROC glGet__iv,
		PFNGLGETSHADERINFOLOGPROC glGet__InfoLog);

void shader_delete(Shader *shader)
{
	if (shader == NULL)
		return;

	glDeleteShader(shader->vertex_shader);
	glDeleteShader(shader->fragment_shader);
	glDeleteProgram(shader->program);
	
	free(shader);
}

Shader *shader_create(const char *vertex_file, const char *fragment_file)
{
	GLint link_status;
	Shader *shader = NULL;

	shader = malloc(sizeof(Shader)); /* FIXME: Could be NULL */

	shader->program = glCreateProgram();
	if (shader->program == 0)
	{
		fprintf(stderr, "Couldn't create GL program\n");
		goto errorout;
	}

	shader->vertex_shader = shader_load(vertex_file, GL_VERTEX_SHADER);
	shader->fragment_shader = shader_load(fragment_file, GL_FRAGMENT_SHADER);
	if (shader->vertex_shader == 0 || shader->fragment_shader == 0)
	{
		fprintf(stderr, "Error loading shaders\n");
		goto errorout;
	}
	glAttachShader(shader->program, shader->vertex_shader);
	glAttachShader(shader->program, shader->fragment_shader);
	glLinkProgram(shader->program);

	glGetProgramiv(shader->program, GL_LINK_STATUS, &link_status);
	if (link_status == GL_FALSE)
	{
		fprintf(stderr, "Error linking shader program\n");
		show_info_log(shader->program, glGetProgramiv, glGetProgramInfoLog);
		goto errorout;
	}
	glBindFragDataLocation(shader->program, 0, "fragColour");

	return shader;

errorout:
	shader_delete(shader);
	return NULL;
}

static GLuint shader_load(const char *file, GLenum type)
{
	GLuint shader = 0;
	char *contents = NULL;
	ALLEGRO_FILE *fd = NULL;
	long filesize;
	GLint compile_status;

	if ((shader = glCreateShader(type)) == 0)
	{
		fprintf(stderr, "Failed to create shader\n");
		goto errorout;
	}

	if ((fd = al_fopen(file, "rb")) == NULL)
	{
		fprintf(stderr, "Couldn't open file: %s\n", file);
		goto errorout;
	}

	if ((filesize = al_fsize(fd)) == -1)
	{
		fprintf(stderr, "Couldn't determine size of file: %s\n", file);
		goto errorout;
	}

	if ((contents = malloc(filesize + 1)) == NULL)
	{
		fprintf(stderr, "Couldn't allocate %ld bytes of memory\n", filesize + 1);
		goto errorout;
	}

	if ((long) al_fread(fd, contents, filesize) != filesize)
	{
		fprintf(stderr, "Error reading shader file %s\n", file);
		goto errorout;
	}
	al_fclose(fd);
	fd = NULL;

	contents[filesize] = '\0';

	glShaderSource(shader, 1, (const GLchar **) &contents, NULL);
	free(contents);
	contents = NULL;

	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == GL_FALSE)
	{
		fprintf(stderr, "Error compiling shader\n");
		show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
		goto errorout;
	}

	return shader;
errorout:
	glDeleteShader(shader);
	free(contents);
	if(fd)
		al_fclose(fd);
	return 0;
}

static void show_info_log(GLuint object, PFNGLGETSHADERIVPROC glGet__iv,
		PFNGLGETSHADERINFOLOGPROC glGet__InfoLog)
{
	GLint log_len;
	char *log;

	glGet__iv(object, GL_INFO_LOG_LENGTH, &log_len);
	log = malloc(log_len);
	glGet__InfoLog(object, log_len, NULL, log);
	fprintf(stderr, "%s", log);
	free(log);
	return;
}
