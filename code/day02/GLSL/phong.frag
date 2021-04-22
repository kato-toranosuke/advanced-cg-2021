#version 150 core

// TODO: add "in" variables
in vec3 eNormal;
in vec3 eViewDir;

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
	float dotProd = max(dot(normalize(eNormal), normalize(eLightDir)), 0.0);
	vec3 diffuse = dotProd * lightColor * diffuseCoeff;
	
	// 鏡面反射光の計算
	vec3 eRef = reflect(-eLightDir, eNormal);
	float cosGamma = max(dot(normalize(eRef), normalize(eViewDir)), 0.0);
	vec3 specular = pow(cosGamma, 20.0) * lightColor * (shininess/50.f);
	
	fragColor = vec4(ambient + diffuse + specular, 1);
}
