#version 330 core

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;

in vec3 in_position;

void main(void)
{
	gl_Position = projection_matrix * view_matrix * model_matrix * vec4(in_position, 1);
}
