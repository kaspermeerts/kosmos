#version 330 core

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;

in vec2 aPosition;
in vec4 aColour;

out vec4 vColour;

void main(void)
{
	gl_Position = uProj * uView * uModel * vec4(aPosition, 0, 1);
	vColour = aColour;
}
