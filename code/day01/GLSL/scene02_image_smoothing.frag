#version 150 core

in vec2 outTexCoord;
out vec4 fragColor;

// TODO: uncomment these lines
uniform sampler2D tex;
uniform int halfKernelSize;
uniform float uScale;
uniform float vScale;

void main()
{
	// TODO: write an appropriate code here
	// change from texture2D() to texture()
	// https://stackoverflow.com/questions/26266198/glsl-invalid-call-of-undeclared-identifier-texture2d

	if(halfKernelSize == 0){
		fragColor = texture(tex, outTexCoord);
	}else {
		float sum_weight = 0; // ガウス関数で計算した重みの総和
		for(int i = -halfKernelSize; i <= halfKernelSize; i++){
			for(int j = -halfKernelSize; j<= halfKernelSize; j++){
				// ガウス関数の重みを計算
				float x = i / halfKernelSize, y = j / halfKernelSize;
				float d = x * x + y * y;
				float weight = exp(-d/(0.5f*0.5f));

				vec2 targetTexCoord = vec2(outTexCoord.s + uScale*i, outTexCoord.t + vScale*j);
				fragColor += weight * texture(tex, targetTexCoord); 
				sum_weight += weight;
			}
		}
		fragColor /= sum_weight;
	}
}
