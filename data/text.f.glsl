#version 330 core

uniform sampler2D font_texture;

in vec2 texcoord;

out vec4 out_colour;

void main(void)
{
	vec4 red = texture2D(font_texture, texcoord);
	out_colour = red.rrrr;
}
