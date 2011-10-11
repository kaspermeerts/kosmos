#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <ralloc.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "font.h"
#include "log.h"
#include "shader.h"

static FT_Library fontlib = NULL;

static void fontlib_destroy(void)
{
	if (fontlib)
		FT_Done_FreeType(fontlib);
}

Font *font_load(const char *filename)
{
	Font *font;

	if (fontlib == NULL)
	{
		if (FT_Init_FreeType(&fontlib) != 0)
		{
			log_err("Error initializing FreeType 2\n");
			return NULL;
		}
		atexit(fontlib_destroy);
	}
	if ((font = ralloc(NULL, Font)) == NULL)
	{
		log_err("Out of memory\n");
		return NULL;
	}
	if (FT_New_Face(fontlib, filename, 0, &font->face) != 0)
	{
		log_err("Error initializing font %s\n", filename);
		ralloc_free(font);
		return NULL;
	}
	log_dbg("Loaded font with %ld glyphs\n", font->face->num_glyphs);

	return font;
}

void font_destroy(Font *font)
{
	FT_Done_Face(font->face);
	ralloc_free(font);
}

static const unsigned int utf8_tail_length[256] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x0F */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x1F */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x2F */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x3F */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x4F */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x5F */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x6F */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x7F */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x8F */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x9F */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xAF */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xBF */
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0xCF */
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0xDF */
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* 0xEF */
3,3,3,3,3,3,3,3,4,4,4,4,5,5,0,0, /* 0xFF */
};

static uint32_t utf8_next(const uint8_t **s)
{
	uint32_t c;
	int i, tail_len;

	/* Start with the first byte */
	c = (*s)[0];
	(*s)++;

	/* Return early if it's plain ASCII */
	if (c < 0x80)
		return c;

	/* Store the first 1-6 bits */
	tail_len = utf8_tail_length[c & 0xff];
	c &= (0x3f >> tail_len);

	/* Main decoding loop */
	for (i = 0; i < tail_len; i++)
	{
		if ((*s)[i] == '\0' || ((*s)[i] & 0xc0) != 0x80)
			break; /* End of string or invalid continuation byte */

		c = (c << 6) + ((*s)[i] & 0x3f);
	}

	*s += i;
	if (i != tail_len)
		return 0xFFFD; /* Replacement character � */

	/* TODO: Check for overlong encodings, surrogate pairs etc */

	return c;
}

static void glyphstring_create(FT_Face face, Text *text, FT_Glyph *glyph_string,
		FT_Vector *pos)
{
	const uint8_t *string = text->string;
	FT_Bool has_kerning;
	FT_UInt glyph_index, previous;
	FT_Vector pen, delta;
	uint32_t charcode;
	int i;

	has_kerning = FT_HAS_KERNING(face);
	previous = 0;
	i = 0;
	pen.x = pen.y = 0;
	while (string[0] != '\0')
	{
		charcode = utf8_next(&string);
		glyph_index = FT_Get_Char_Index(face, charcode);
		if (has_kerning && previous && glyph_index)
			FT_Get_Kerning(face, previous, glyph_index, FT_KERNING_DEFAULT,
					&delta);
		else
			delta.x = 0;

		if (glyph_index == 0)
			log_err("Glyph for character U+%X missing\n", charcode);

		if (FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER) != 0)
		{
			log_err("Error loading glyph for character U+%X\n", charcode);
			continue;
		}
		if (FT_Get_Glyph(face->glyph, &glyph_string[i]) != 0)
		{
			log_err("Error copying glyph for character U+%X\n", charcode);
			continue;
		}

		pen.x += delta.x;

		pos[i] = pen;

		pen.x += face->glyph->advance.x;
		pen.y += face->glyph->advance.y;

		previous = glyph_index;
		i++;
	}

	text->num_glyphs = i;
}

static void compute_glyphstring_bbox(FT_Glyph *string, FT_Vector *pos,
		int len, FT_BBox *bbox)
{
	FT_BBox glyph_bbox;
	int i;

	bbox->xMin = bbox->yMin = 32000;
	bbox->xMax = bbox->yMax = -32000;

	for (i = 0; i < len; i++)
	{
		FT_Glyph_Get_CBox(string[i], FT_GLYPH_BBOX_GRIDFIT, &glyph_bbox);

		glyph_bbox.xMin += pos[i].x;
		glyph_bbox.xMax += pos[i].x;
		glyph_bbox.yMin += pos[i].y;
		glyph_bbox.yMax += pos[i].y;

		if (glyph_bbox.xMin < bbox->xMin)
			bbox->xMin = glyph_bbox.xMin;

		if (glyph_bbox.xMax > bbox->xMax)
			bbox->xMax = glyph_bbox.xMax;

		if (glyph_bbox.yMin < bbox->yMin)
			bbox->yMin = glyph_bbox.yMin;

		if (glyph_bbox.yMax > bbox->yMax)
			bbox->yMax = glyph_bbox.yMax;
	}

	if (bbox->xMin > bbox->xMax || bbox->yMin > bbox->yMax)
	{
		bbox->xMin = 0;
		bbox->xMax = 0;
		bbox->yMin = 0;
		bbox->yMax = 0;
	}
}

static void blit_glyph(FT_BitmapGlyph glyph, GLubyte *image, int x, int y,
		int width, int height)
{
	FT_Bitmap *bitmap = &glyph->bitmap;
	int startx, starty, imagex, imagey, ix, iy;

	/* The bitmap doesn't contain the complete glyph, only the nonzero bytes */
	startx = x + glyph->left;
	starty = y + glyph->top - bitmap->rows;

	for (iy = 0; iy < bitmap->rows; iy++)
	{
		for (ix = 0; ix < bitmap->width; ix++)
		{
			/* The glyph is stored upside-down */
			GLubyte b = bitmap->buffer[(bitmap->rows - 1 - iy)*bitmap->pitch
					+ ix];
			imagex = startx + ix;
			imagey = starty + iy;

			/* This won't happen but let's make sure */
			if (imagex < 0 || imagex > width || imagey < 0 || imagey > height)
				continue;

			if (image[imagey*width + imagex] == 0)
				image[imagey*width + imagex] =  b;
		}
	}
}

Text *text_create(Font *font, const uint8_t *string, int size)
{
	Text *text;
	FT_Face face = font->face;
	FT_Glyph *glyph_string;
	FT_Vector *pos;
	FT_BBox bbox;
	FT_Long width, height;
	size_t len;
	float x, y;
	int i;

	/* XXX Remove this */
	if (string == NULL)
		string = (const uint8_t *) "";

	if (FT_Set_Char_Size(face, 0, size*64, 0, 0) != 0)
	{
		log_err("Error setting character size\n");
		return NULL;
	}

	text = ralloc(font, Text);
	if (text == NULL)
	{
		log_err("Out of memory\n");
		return NULL;
	}
	text->size = size;
	text->vao = text->vbo = text->texture = 0;
	text->texture_image = NULL;
	text->string = (uint8_t *) ralloc_strdup(text, (const char *) string);
	len = strlen((const char *) string); /* Bytecount */
	/* We allocate more space than necessary for the glyph string, better safe
	 * than sorry. */
	glyph_string = ralloc_array(text, FT_Glyph, len);
	pos = ralloc_array(text, FT_Vector, len);
	if (text->string == NULL || glyph_string == NULL || pos == NULL)
	{
		log_err("Out of memory\n");
		ralloc_free(text);
		return NULL;
	}

	/* The UTF-8 bytestring is converted to an array of glyphs */
	glyphstring_create(face, text, glyph_string, pos);
	/* We determine how big the text will be. This information is used
	 * to compute the size of the texture we'll store the text in */
	compute_glyphstring_bbox(glyph_string, pos, text->num_glyphs, &bbox);

	width  = bbox.xMax - bbox.xMin;
	height = bbox.yMax - bbox.yMin;
	text->width = (width/64 + 0x3) & ~0x3; /* Align to 4 bytes */
	text->height = height/64;
	text->texture_image = rzalloc_array(text, GLubyte,
			text->width * text->height);
	if (text->texture_image == NULL)
	{
		log_err("Out of memory\n");
		for (i = 0; i < text->num_glyphs; i++)
			FT_Done_Glyph(glyph_string[i]);
		ralloc_free(text);
		return NULL;
	}
	/* Now we can render the text to a texture */
	for (i = 0; i < text->num_glyphs; i++)
	{
		FT_Glyph glyph;
		FT_BitmapGlyph bitmap_glyph;
		FT_Vector pen;

		glyph = glyph_string[i];
		pen.x = pos[i].x;
		pen.y = pos[i].y;

		/* Render the new glyph and destroy the old one */
		if (FT_Glyph_To_Bitmap(&glyph, FT_LOAD_TARGET_NORMAL, &pen, 1) != 0)
		{
			log_err("Error rendering glyph to bitmap for character '%c'\n",
					string[i]);
			FT_Done_Glyph(glyph_string[i]);
			continue;
		}

		bitmap_glyph = (FT_BitmapGlyph) glyph;
		blit_glyph(bitmap_glyph, text->texture_image, (pen.x - bbox.xMin)/64,
				(pen.y - bbox.yMin)/64, text->width, text->height);
		FT_Done_Glyph(glyph);
	}
	ralloc_free(glyph_string);
	ralloc_free(pos);

	x = bbox.xMin/64;
	y = bbox.yMin/64;

	/* Why do we add text->width and not bbox.xMax? Because OpenGL wants
	 * textures aligned at 4 bytes. This fixes a very subtle bug with 
	 * a very noticeable effect */
	text->vertex[0].x = x;
	text->vertex[0].y = y;
	text->vertex[0].u = 0;
	text->vertex[0].v = 0;

	text->vertex[1].x = x + text->width;
	text->vertex[1].y = y;
	text->vertex[1].u = 1;
	text->vertex[1].v = 0;

	text->vertex[2].x = x + text->width;
	text->vertex[2].y = y + text->height;
	text->vertex[2].u = 1;
	text->vertex[2].v = 1;

	text->vertex[3].x = x;
	text->vertex[3].y = y + text->height;
	text->vertex[3].u = 0;
	text->vertex[3].v = 1;

	text->num_vertices = 4;
	text->colour[0] = 0.0;
	text->colour[1] = 0.0;
	text->colour[2] = 1.0;

	return text;
}

void text_upload_to_gpu(Shader *shader, Text *text)
{
	glGenVertexArrays(1, &text->vao);
	glBindVertexArray(text->vao);

	glGenBuffers(1, &text->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, text->vbo);
	glBufferData(GL_ARRAY_BUFFER, text->num_vertices*sizeof(struct TextVertex),
			text->vertex, GL_STATIC_DRAW);
	glEnableVertexAttribArray(shader->location[SHADER_ATT_POSITION]);
	glVertexAttribPointer(shader->location[SHADER_ATT_POSITION], 2, GL_FLOAT,
			GL_FALSE, sizeof(struct TextVertex),
			(void *) offsetof(struct TextVertex, x));
	glEnableVertexAttribArray(shader->location[SHADER_ATT_TEXCOORD]);
	glVertexAttribPointer(shader->location[SHADER_ATT_TEXCOORD], 2, GL_FLOAT,
			GL_FALSE, sizeof(struct TextVertex),
			(void *) offsetof(struct TextVertex, u));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenTextures(1, &text->texture);
	glBindTexture(GL_TEXTURE_2D, text->texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, text->width, text->height, 
			0, GL_RED, GL_UNSIGNED_BYTE, text->texture_image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);

	ralloc_free(text->texture_image);
}

void text_render(Shader *shader, Text *text, int x, int y)
{
	glUniform1i(glGetUniformLocation(shader->program, "text_texture"), 0);
	glUniform3fv(glGetUniformLocation(shader->program, "text_colour"),
		1, text->colour);
	glUniform2i(glGetUniformLocation(shader->program, "text_location"), x, y);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, text->texture);
	glBindVertexArray(text->vao);
	glDrawArrays(GL_QUADS, 0, text->num_vertices);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void text_destroy(Text *text)
{
	glDeleteVertexArrays(1, &text->vao);
	glDeleteBuffers(1, &text->vbo);
	glDeleteTextures(1, &text->texture);

	ralloc_free(text);
}

void text_create_and_render(Shader *shader, Font *font, int x, int y, 
		const char *fmt, ...)
{
	uint8_t *string;
	va_list va;
	Text *text;

	va_start(va, fmt);
	string = (uint8_t *) ralloc_vasprintf(NULL, fmt, va);
	va_end(va);

	if (string == NULL)
		return;

	text = text_create(font, string, 16); /* FIXME Argument */
	ralloc_free(string);
	if (text ==	NULL)
		return;

	text_upload_to_gpu(shader, text);
	text_render(shader, text, x, y);
	text_destroy(text);
}
