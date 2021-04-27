#version 150 core

in vec4 vertexPosition;
in vec3 vertexNormal;

// TODO: define "out" variables
out vec3 eNormal;
out vec3 ePosition;
out vec4 texCoord; // シャドウマップ参照用のテクスチャ座標

// TODO: uncomment these lines
uniform mat4 biasedShadowProjModelView;
uniform mat3 modelViewInverseTransposed;
uniform mat4 projMatrix;
uniform mat4 modelViewMatrix;

void main()
{
	// TODO: rewirte this function
	// [ビュー座標系]法線ベクトルの計算
	// モデルビュー変換や投影行列といった変換行列に非一様スケーリングを含む場合、法線に対しただ変換行列をかけるだけではだめ。→モデルビュー行列の逆行列の転置行列をかければ良い。
	eNormal = modelViewInverseTransposed * vertexNormal;
	
	// [ビュー座標系]位置ベクトルの計算
	ePosition = (modelViewMatrix * vertexPosition).xyz;
	
	// [クリッピング座標系]シャドウマップ参照用のテクスチャ座標を計算
	texCoord = biasedShadowProjModelView * vertexPosition;

	// [クリッピング座標系]頂点座標の計算
	gl_Position = projMatrix * vec4(ePosition, 1);
}