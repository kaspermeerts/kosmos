#version 330 core

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;

in vec3 aPosition;

void main(void)
{
	gl_Position = uProj * uView * uModel * vec4(aPosition, 1);
}
