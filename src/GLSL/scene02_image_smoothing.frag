#version 150 core

in vec2 outTexCoord;
out vec4 fragColor;

// TODO: uncomment these lines
uniform sampler2D tex;
uniform int halfKernelSize;
uniform float uScale;
uniform float vScale;

void main()
{
	// TODO: write an appropriate code here
	// change from texture2D() to texture()
	fragColor = texture(tex, outTexCoord);
}
