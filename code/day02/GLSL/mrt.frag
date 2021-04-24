#version 150 core

// TODO: define "in" variables
in vec3 viewVertexNormal;
in vec4 viewVertexPosition;
in vec4 deviceVertexPosition;

out vec4 fragColors[3];

void main()
{
	fragColors[0] = viewVertexPosition; // 座標
	fragColors[1] = vec4(0.5f * viewVertexNormal + 0.5f, 1.f); // 法線

	float z = 0.5 * deviceVertexPosition.z + 0.5;
	fragColors[2] = vec4(z, z, z, 1); // デプス値
}
