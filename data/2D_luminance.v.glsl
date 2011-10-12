#version 330 core

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;
uniform sampler2D uTexture;

in vec2 aPosition;
in vec4 aColour;
in vec2 aTexCoord;

out vec4 vColour;
out vec2 vTexCoord;

void main(void)
{
	gl_Position = uProj * uView * uModel * vec4(aPosition, 0, 1);
	vColour = aColour;
	vTexCoord = aTexCoord;
}
