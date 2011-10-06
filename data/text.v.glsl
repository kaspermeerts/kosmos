#version 330 core

uniform mat4 projection_matrix;
uniform sampler2D text_texture;
uniform ivec2 text_location;
uniform vec3 text_colour;

in vec2 in_position;
in vec2 in_texcoord;

out vec2 texcoord;

void main(void)
{
	gl_Position = projection_matrix * 
			vec4(in_position.x + text_location.x, 
			     in_position.y + text_location.y, 0, 1);
	texcoord = in_texcoord;
}
