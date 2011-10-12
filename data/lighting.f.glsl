#version 330 core

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;
uniform vec3 light_pos, light_ambient, light_diffuse, light_specular;

in vec3 L, E, vertNormal;

out vec4 oColour;

void main(void)
{
	vec3 N, R;
	float NdotL, RdotE;
	float shininess = 11.264;

	N = normalize(vertNormal);
	R = reflect(-L, N);
	NdotL = max(dot(N, L), 0);
	RdotE = max(dot(R, E), 0);

	oColour = vec4(light_ambient + NdotL * light_diffuse +
			pow(RdotE, shininess) * light_specular, 1);

}
