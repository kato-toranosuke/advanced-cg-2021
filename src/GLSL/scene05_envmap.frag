#version 150 core

#define PI 3.141592653589793

in vec3 vWorldEyeDir;
in vec3 vWorldNormal;

out vec4 fragColor;

// TODO: uncomment these lines
uniform sampler2D envmap;

float atan2(in float y, in float x)
{
    return x == 0.0 ? sign(y)*PI/2 : atan(y, x);
}

void main()
{
	// TODO: write an appropriate code here
	// 正反射ベクトルを計算
	vec3 vWorldRefDir = normalize(reflect(vWorldEyeDir, vWorldNormal));
	// テクスチャ座標の計算
	float theta = asin(vWorldRefDir.y);
	float psi = atan2(vWorldRefDir.z, vWorldRefDir.x);

	// 角度を正規化し、テクスチャ座標に変換
	float normalTheta = (theta + PI/2.f) / PI;
	float normalPsi = psi / PI;

	fragColor = texture(envmap, vec2(normalPsi, normalTheta));
}
