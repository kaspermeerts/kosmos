#ifndef KOSMOS_FONT
#define KOSMOS_FONT

#include <GL/gl.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "shader.h"

typedef struct Font {
	FT_Face face;
} Font;

typedef struct Text {
	uint8_t *string; /* UTF-8 string */
	int num_glyphs;
	Font *font;
	int size;
	GLuint vao;
	GLuint vbo;
	GLuint texture;
	int num_vertices;
	struct TextVertex {
		GLfloat x, y;
		GLfloat u, v;
	} vertex[4];
	int width;
	int height;
	GLubyte *texture_image;
	GLfloat colour[3];
} Text;

Font *font_load(const char *filename);
void font_destroy(Font *font);
Text *text_create(Font *font, const uint8_t *text, int size);
void text_upload_to_gpu(Shader *shader, Text *text);
void text_render(Shader *shader, Text *text, int x, int y);
void text_destroy(Text *text);
void text_create_and_render(Shader *shader, Font *font, int x, int y, const char *fmt, ...);
#endif
