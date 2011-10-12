#version 330 core

uniform mat4 uProj;
uniform mat4 uModel;
uniform mat4 uView;
uniform vec3 light_pos, light_ambient, light_diffuse, light_specular;

in vec3 aPosition;
in vec3 aNormal;

out vec3 L, E, vertNormal;

void main(void)
{
	/* Position in eye space */
	mat4 modelView = uView * uModel;
	vec4 eye_pos = modelView * vec4(aPosition, 1.0);
	vec3 light_dir = light_pos - eye_pos.xyz;
	gl_Position = uProj * eye_pos;

	mat3 normal_matrix = inverse(transpose(mat3(modelView)));
	L = normalize(light_dir);
	E = -normalize(eye_pos.xyz);
	vertNormal = normal_matrix * aNormal;
}


