#version 150 core

in vec3 vVertexNormal;
out vec4 fragColor;

void main()
{
	// TODO: write an appropriate code here
	fragColor = vec4(0.5f * vVertexNormal + 0.5f, 1.f);
}
