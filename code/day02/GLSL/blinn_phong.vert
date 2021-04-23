#version 150 core

in vec4 vertexPosition;
in vec3 vertexNormal;

// TODO: define "out" variables
out vec3 eNormal;
out vec3 ePosition;

// TODO: uncomment this line
uniform mat3 modelViewInverseTransposed;
uniform mat4 modelViewMatrix;
uniform mat4 projMatrix;

void main()
{
	// TODO: rewirte this function
	// 法線ベクトルの計算
	// モデルビュー変換や投影行列といった変換行列に非一様スケーリングを含む場合、法線に対しただ変換行列をかけるだけではだめ。→モデルビュー行列の逆行列の転置行列をかければ良い。
	eNormal = modelViewInverseTransposed * vertexNormal;
	
	// 位置ベクトルの計算
	ePosition = (modelViewMatrix * vertexPosition).xyz;
	
	gl_Position = projMatrix * vec4(ePosition, 1);
}