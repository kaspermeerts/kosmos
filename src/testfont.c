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

	al_create_display(800, 600);

	return 0;
}

int main(int argc, char **argv)
{
	Text *text;
	Font *font;
	Shader *shader;
	const char *string;

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

	font = font_load(STRINGIFY(ROOT_PATH) "/data/DejaVuLGCSans.ttf", 12);
	if (font == NULL)
		return 1;
	text = text_create(font, string, 200, 200);

	text_upload_to_gpu(shader, text);

	glmProjectionMatrix = glmNewMatrixStack();
	glmLoadIdentity(glmProjectionMatrix);
	glmOrtho(glmProjectionMatrix, 0, 800, 0, 600, -1, 1);

	glClearColor(0, 0, 0.4, 1.0);
	while(1)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glmUniformMatrix(shader->location[SHADER_UNI_P_MATRIX], glmProjectionMatrix);
		text_render(shader, text);
		al_flip_display();
	}

	return 0;
}
