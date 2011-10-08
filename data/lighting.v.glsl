#version 330 core

uniform mat4 projection_matrix;
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform vec3 light_pos, light_ambient, light_diffuse, light_specular;

in vec3 in_position;
in vec3 in_normal;

out vec3 L, E, vertNormal;

void main(void)
{
	/* Position in eye space */
	mat4 modelview_matrix = view_matrix * model_matrix;
	vec4 eye_pos = modelview_matrix * vec4(in_position, 1.0); 
	vec3 light_dir = light_pos - eye_pos.xyz;
	gl_Position = projection_matrix * eye_pos;

	mat3 normal_matrix = inverse(transpose(mat3(modelview_matrix)));
	L = normalize(light_dir);
	E = -normalize(eye_pos.xyz);
	vertNormal = normal_matrix * in_normal;
}


