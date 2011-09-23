#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <allegro5/allegro.h>

#include "shader.h"
#include "log.h"

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
	ALLEGRO_PATH *vpath, *fpath;
	GLint link_status;
	Shader *shader = NULL;

	/* 0 is a sane default for both shaders and the program */
	if ((shader = calloc(1, sizeof(Shader))) == NULL)
		return NULL;
	
	shader->program = glCreateProgram();
	if (shader->program == 0)
	{
		/* How the hell can this fail */
		log_err("Couldn't create GL program\n");
		goto errorout;
	}

	shader->vertex_shader = shader_load(vertex_file, GL_VERTEX_SHADER);
	shader->fragment_shader = shader_load(fragment_file, GL_FRAGMENT_SHADER);
	if (shader->vertex_shader == 0 || shader->fragment_shader == 0)
	{
		log_err("Error loading shaders\n");
		goto errorout;
	}
	glAttachShader(shader->program, shader->vertex_shader);
	glAttachShader(shader->program, shader->fragment_shader);
	glLinkProgram(shader->program);

	glGetProgramiv(shader->program, GL_LINK_STATUS, &link_status);
	if (link_status == GL_FALSE)
	{
		log_err("Error linking shader program\n");
		show_info_log(shader->program, glGetProgramiv, glGetProgramInfoLog);
		goto errorout;
	}
	glBindFragDataLocation(shader->program, 0, "out_colour");

	vpath = al_create_path(vertex_file);
	fpath = al_create_path(fragment_file);
	log_dbg("Loaded shader from %s and %s\n", al_get_path_filename(vpath),
			al_get_path_filename(fpath));
	al_destroy_path(fpath);
	al_destroy_path(vpath);

	show_info_log(shader->program, glGetProgramiv, glGetProgramInfoLog);

	shader->location[SHADER_ATT_POSITION] =
			glGetAttribLocation(shader->program, "in_position");
	shader->location[SHADER_ATT_NORMAL] =
			glGetAttribLocation(shader->program, "in_normal");
	shader->location[SHADER_UNI_M_MATRIX] =
			glGetUniformLocation(shader->program, "model_matrix");
	shader->location[SHADER_UNI_V_MATRIX] =
			glGetUniformLocation(shader->program, "view_matrix");
	shader->location[SHADER_UNI_P_MATRIX] =
			glGetUniformLocation(shader->program, "projection_matrix");
	shader->location[SHADER_UNI_LIGHT_POS] =
			glGetUniformLocation(shader->program, "light_pos");
	shader->location[SHADER_UNI_LIGHT_AMBIENT] =
			glGetUniformLocation(shader->program, "light_ambient");
	shader->location[SHADER_UNI_LIGHT_DIFFUSE] =
			glGetUniformLocation(shader->program, "light_diffuse");
	shader->location[SHADER_UNI_LIGHT_SPECULAR] =
			glGetUniformLocation(shader->program, "light_specular");
	shader->location[SHADER_UNI_LIGHT_SHININESS] =
			glGetUniformLocation(shader->program, "shininess");


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
		log_err("Failed to create shader\n");
		goto errorout;
	}

	if ((fd = al_fopen(file, "rb")) == NULL)
	{
		log_err("Couldn't open file: %s\n", file);
		goto errorout;
	}

	if ((filesize = al_fsize(fd)) < 0)
	{
		log_err("Couldn't determine size of file: %s\n", file);
		goto errorout;
	}

	if ((contents = malloc((size_t) filesize + 1)) == NULL)
	{
		log_err("Couldn't allocate %ld bytes of memory\n", filesize + 1);
		goto errorout;
	}

	if (al_fread(fd, contents, (size_t) filesize) != (size_t) filesize)
	{
		log_err("Error reading shader file %s\n", file);
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
		log_err("Error compiling shader\n");
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
	if (log_len <= 1)
		return;
	log = malloc((size_t) log_len);
	glGet__InfoLog(object, log_len, NULL, log);
	log_err("Info log:\n%s", log);
	free(log);
	return;
}
