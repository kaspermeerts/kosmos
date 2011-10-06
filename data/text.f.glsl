#version 330 core

uniform mat4 projection_matrix;
uniform sampler2D text_texture;
uniform ivec2 text_location;
uniform vec3 text_colour;

in vec2 texcoord;

out vec4 out_colour;

void main(void)
{
	vec4 luminance = texture2D(text_texture, texcoord);
	out_colour = vec4(text_colour*luminance.rrr, luminance.r);
}
