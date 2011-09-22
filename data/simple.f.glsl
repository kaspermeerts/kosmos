#version 330 core

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform vec3 light_pos, light_ambient, light_diffuse, light_specular;
uniform float shininess;

in vec3 L, E, vertNormal;

out vec4 fragColour;

void main(void)
{
	vec3 N, R;
	float NdotL, RdotE;

	N = normalize(vertNormal);
	R = reflect(-L, N);
	NdotL = max(dot(N, L), 0);
	RdotE = max(dot(R, E), 0);

	fragColour = vec4(light_ambient + NdotL * light_diffuse +
			pow(RdotE, shininess) * light_specular * 0, 1);

}


