#version 330 core

uniform sampler2D font_texture;
uniform mat4 projection_matrix;

in vec2 in_position;
in vec2 in_texcoord;

out vec2 texcoord;

void main(void)
{
	gl_Position = projection_matrix * vec4(in_position, 0, 1);
	texcoord = in_texcoord;
}
