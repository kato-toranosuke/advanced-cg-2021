#version 150 core

in vec4 vertexPosition;
in vec3 vertexNormal;

// TODO: define "out" variables
out vec3 viewVertexNormal; // ビュー座標系（カメラ座標系）における法線ベクトル
out vec4 viewVertexPosition; // カメラ座標系における座標
out vec4 deviceVertexPosition; //正規化デバイス座標系における座標

uniform mat4 projMatrix;
uniform mat4 modelViewMatrix;
// add
uniform mat3 modelViewInvTransposed;

void main()
{
	// TODO: rewrite this function
	gl_Position = projMatrix * modelViewMatrix * vertexPosition;

	viewVertexNormal = normalize(modelViewInvTransposed * vertexNormal);
	viewVertexPosition = modelViewMatrix * vertexPosition;
	deviceVertexPosition = projMatrix * viewVertexPosition;
}