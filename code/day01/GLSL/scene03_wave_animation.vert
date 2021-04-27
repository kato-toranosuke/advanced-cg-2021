#version 150 core

in vec4 vertexPosition;

// TODO: uncomment these lines
uniform float temporalSignal;
uniform mat4 projModelViewMatrix;

void main()
{
	// TODO: write an appropriate code here
	float initY = (sin(vertexPosition.x*2)+cos(vertexPosition.z*2))*0.8f;
	float scale = (cos(vertexPosition.x*2)+sin(vertexPosition.z*2))/2.0f*cos(temporalSignal*4);
	vec4 animVPosition = vec4(vertexPosition.x, initY*scale, vertexPosition.z, vertexPosition.w);
	gl_Position = projModelViewMatrix * animVPosition;
}
