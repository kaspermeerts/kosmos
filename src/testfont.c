#include <stdio.h>
#include <allegro5/allegro.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include "glm.h"
#include "font.h"
#include "shader.h"

#define STRINGIFY(s) XSTRINGIFY(s)
#define XSTRINGIFY(x) #x

static int init_allegro(void)
{
	if (!al_init())
		return 1;
	
	al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE |
			ALLEGRO_OPENGL | ALLEGRO_OPENGL_3_0 );	
	al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);

	al_create_display(400, 400);

	return 0;
}

int main(int argc, char **argv)
{
	Text *text[6];
	Font *font;
	Shader *shader;
	const char *string;
	Matrix proj;
	int i;

	if (argc < 2)
		string = "Kasper is groot!";
	else
		string = argv[1];

	init_allegro();
	glewInit();

	shader = shader_create(STRINGIFY(ROOT_PATH) "/data/text.v.glsl",
	                       STRINGIFY(ROOT_PATH) "/data/text.f.glsl");
	if (shader == NULL)
		return 1;
	glUseProgram(shader->program);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	font = font_load(STRINGIFY(ROOT_PATH) "/data/DejaVuLGCSans.ttf", 36);
	if (font == NULL)
		return 1;
	text[0] = text_create(font, (const uint8_t *) "Latin");
	text[1] = text_create(font, (const uint8_t *) "Ελληνικό αλφάβητο");
	text[2] = text_create(font, (const uint8_t *) "кириллица");
	text[3] = text_create(font, (const uint8_t *) "∀☭∈R: ☭ ≥ ☭");
	text[4] = text_create(font, (const uint8_t *) "�");
	text[5] = text_create(font, (const uint8_t *) string);


	for (i = 0; i < 6; i++)
		text_upload_to_gpu(shader, text[i]);

	glClearColor(0xED/255., 0xEB/255., 0xEC/255., 1.0);
	while(1)
	{
		glmLoadIdentity(&proj);
		glmOrtho(&proj, 0, 400, 0, 400, -1, +1);
		glmUniformMatrix(shader->location[SHADER_UNI_P_MATRIX], &proj);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (i = 0; i < 6; i++)
		{
			int y = 400 - (font->size * 1.5 * 76/72)*(i+1);
			text_render(shader, text[i], 20, y);
		}
		al_flip_display();
	}
	
	for (i = 0; i < 6; i++)
		text_destroy(text[i]);
	font_destroy(font);

	return 0;
}
