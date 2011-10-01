#ifndef KOSMOS_FONT
#define KOSMOS_FONT

#include <GL/gl.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "shader.h"

typedef struct Colour {
	GLubyte r, g, b, a;
} Colour;

typedef struct Font {
	FT_Face face;
	int size;
} Font;

typedef struct Text {
	const char *string;
	size_t length;
	Font *font;
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
} Text;

Font *font_load(const char *filename, int size);
Text *text_create(Font *font, const char *text, int x, int y);
void text_upload_to_gpu(Shader *shader, Text *text);
void text_render(Shader *shader, Text *text);
#endif
