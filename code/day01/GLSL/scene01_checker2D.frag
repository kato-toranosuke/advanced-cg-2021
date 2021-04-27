#version 150 core

in vec2 outTexCoord;
out vec4 fragColor;

// TODO: uncomment these lines
uniform vec4 checkerColor0;
uniform vec4 checkerColor1;
uniform vec2 checkerScale;

void main()
{
	// TODO: write an appropriate code here
	float s_ration = mod(outTexCoord.s, checkerScale.s)/checkerScale.s;
	float t_ration = mod(outTexCoord.t, checkerScale.t)/checkerScale.t;
	if((s_ration < 0.5f && t_ration < 0.5f) || (s_ration >= 0.5 && t_ration >= 0.5)){
		fragColor = checkerColor0;
	} else {
		fragColor = checkerColor1;
	}

}
