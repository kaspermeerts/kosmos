#include <stdio.h>
#include <allegro5/allegro.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include "glm.h"
#include "font.h"
#include "shader.h"
#include "util.h"

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
	int i;

	if (argc < 2)
		string = "Kasper is groot!";
	else
		string = argv[1];

	init_allegro();
	glewInit();

	shader = shader_create(STRINGIFY(ROOT_PATH) "/data/2D_luminance.v.glsl",
	                       STRINGIFY(ROOT_PATH) "/data/2D_luminance.f.glsl");
	if (shader == NULL)
		return 1;
	glUseProgram(shader->program);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glmProjectionMatrix = glmNewMatrixStack();
	glmViewMatrix = glmNewMatrixStack();
	glmModelMatrix = glmNewMatrixStack();

	font = font_load(STRINGIFY(ROOT_PATH) "/data/DejaVuLGCSans.ttf");
	if (font == NULL)
		return 1;
	text[0] = text_create(font, "Latin", 14);
	text[1] = text_create(font, "Ελληνικό αλφάβητο", 14);
	text[2] = text_create(font, "кириллица", 14);
	text[3] = text_create(font, "∀☭∈R: ☭ ≥ ☭", 14);
	text[4] = text_create(font, "�", 14);
	text[5] = text_create(font, string, 14);


	for (i = 0; i < 6; i++)
		text_upload_to_gpu(shader, text[i]);

	glClearColor(0xED/255., 0xEB/255., 0xEC/255., 1.0);
	while(1)
	{
		glmLoadIdentity(glmProjectionMatrix);
		glmOrtho(glmProjectionMatrix, 0, 400, 0, 400, -1, +1);
		glmUniformMatrix(shader->location[SHADER_UNI_P_MATRIX], glmProjectionMatrix);
		glmLoadIdentity(glmViewMatrix);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (i = 0; i < 6; i++)
		{
			int y = 400 - (text[i]->size * 1.5 * 76/72)*(i+1);
			glmLoadIdentity(glmModelMatrix);
			glmTranslate(glmModelMatrix, 20, y, 0);
			text_render(shader, text[i]);
		}
		al_flip_display();
	}

	for (i = 0; i < 6; i++)
		text_destroy(text[i]);
	font_destroy(font);

	return 0;
}
