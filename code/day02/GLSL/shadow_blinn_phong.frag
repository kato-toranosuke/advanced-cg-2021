#version 150 core

// TODO: define "in" variables
in vec3 eNormal;
in vec3 ePosition;
in vec4 texCoord; // シャドウマップ参照用のテクスチャ座標

out vec4 fragColor;

// TODO: uncomment this line
uniform sampler2DShadow shadowTex;
// TODO: define uniform variables
uniform vec3 eLightDir;
uniform vec3 lightColor;
uniform vec3 diffuseCoeff;
uniform vec3 ambient;
uniform float shininess;

void main()
{
	// TODO: rewirte this function
	// 拡散反射光の計算
	vec3 eNormalNormalized = normalize(eNormal);
	float dotDiffuse = max(dot(eNormalNormalized, eLightDir), 0.0);
	vec3 diffuseColor = dotDiffuse * lightColor * diffuseCoeff;

	// 鏡面反射光の計算
	vec3 specularColor = vec3(0, 0, 0);
	if (dotDiffuse > 0.0) {
		vec3 viewingDir = normalize(-ePosition);
		vec3 halfVec = normalize(eLightDir + viewingDir);
		float dotSpecular = max(dot(eNormalNormalized, halfVec), 0.0);
		specularColor = pow(dotSpecular, shininess) * lightColor * diffuseCoeff;
	}

	fragColor = vec4(ambient + diffuseColor + specularColor, 1) * textureProj(shadowTex, texCoord);
}
