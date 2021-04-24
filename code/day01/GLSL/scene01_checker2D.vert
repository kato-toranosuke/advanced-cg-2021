#version 150 core

in vec4 vertexPosition;
in vec2 inTexCoord;

out vec2 outTexCoord;

void main()
{
	// TODO: write an appropriate code here
	// gl_Positon: バーテックスシェーダの出力変数
	gl_Position = vertexPosition;

	outTexCoord = inTexCoord;
}
