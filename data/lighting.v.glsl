#version 330 core

uniform mat4 uProj;
uniform mat4 uModel;
uniform mat4 uView;
uniform vec3 light_pos, light_ambient, light_diffuse, light_specular;

in vec3 aPosition;
in vec3 aNormal;

out vec3 vNormal;
out vec3 L, E;

void main(void)
{
	mat4 modelView = uView * uModel;
	mat3 normal_matrix = inverse(transpose(mat3(modelView)));
	vec4 eye_pos = modelView * vec4(aPosition, 1.0);
	vec3 light_dir = light_pos - eye_pos.xyz;
	gl_Position = uProj * eye_pos;

	L = normalize(light_dir);
	E = -normalize(eye_pos.xyz);
	vNormal = normal_matrix * aNormal;
}


