#version 330 core

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;
uniform vec3 light_pos, light_ambient, light_diffuse, light_specular;

in vec3 vNormal;
in vec3 L, E;

out vec4 oColour;

void main(void)
{
	vec3 N, R, ambient, diffuse, specular;
	const float shininess = 11.264;
	float intensity;

	N = normalize(vNormal);
	R = reflect(-L, N);

	ambient = light_ambient;
	diffuse = max(dot(N, L), 0) * light_diffuse;
	specular = pow(max(dot(R, E), 0), shininess) * light_specular;
	oColour = vec4(ambient + diffuse + specular, 1);

}
