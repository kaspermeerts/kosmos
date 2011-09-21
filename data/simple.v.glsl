#version 330 core

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform vec3 light_dir, light_ambient, light_diffuse, light_specular;
uniform float shininess;

in vec4 in_position;
in vec3 in_normal;

out vec3 L, E, vertNormal;

void main(void)
{
	vec4 eye_pos = modelview_matrix * in_position;
	gl_Position = projection_matrix * eye_pos;

	mat3 normal_matrix = inverse(transpose(mat3(modelview_matrix)));
	L = normalize(mat3(modelview_matrix) * light_dir);
	E = -normalize(eye_pos.xyz);
	vertNormal = normal_matrix * in_normal;
}


