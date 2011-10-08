#version 330 core

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform vec3 light_pos, light_ambient, light_diffuse, light_specular;

in vec3 L, E, vertNormal;

out vec4 out_colour;

void main(void)
{
	vec3 N, R;
	float NdotL, RdotE;
	float shininess = 11.264;

	N = normalize(vertNormal);
	R = reflect(-L, N);
	NdotL = max(dot(N, L), 0);
	RdotE = max(dot(R, E), 0);

	out_colour = vec4(light_ambient + NdotL * light_diffuse +
			pow(RdotE, shininess) * light_specular, 1);

}
