#version 330 core

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;
uniform sampler2D uTexture;

in vec4 vColour;
in vec2 vTexCoord;

out vec4 oColour;

void main(void)
{
	float luminance = texture2D(uTexture, vTexCoord).r;
	oColour = vColour*luminance;
}
