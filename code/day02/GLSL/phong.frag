#version 150 core

// TODO: add "in" variables
in vec3 eNormal;
in vec3 ePosition;

out vec4 fragColor;

// TODO: add uniform variables
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
	if (dotDiffuse > 0.0){
		vec3 viewingDir = normalize(-ePosition);
		vec3 eRefDir = normalize(reflect(-eLightDir, eNormalNormalized));
		float dotSpecular = max(dot(viewingDir, eRefDir), 0.0);
		specularColor = pow(dotSpecular, shininess) * lightColor * diffuseCoeff;
	}

	fragColor = vec4(ambient + diffuseColor + specularColor, 1);
}
