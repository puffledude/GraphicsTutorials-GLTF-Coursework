#version 330 core

uniform sampler2D sceneTex;
uniform float time;
uniform int isVertical;


const float scaleFactors[7] = float[](
	0.006,
	0.061,
	0.242,
	0.383,
	0.242,
	0.061,
	0.006
);


in Vertex{
	vec2 texCoord;
} IN;

out vec4 fragColour;

vec4 blur(sampler2D sceneTex, vec2 texCoord, float time)
{
	vec4 computed = vec4(0, 0, 0, 1);
	vec2 delta = vec2(0,0);

	if (isVertical == 1)
		delta = dFdy(IN.texCoord); // Vertical direction
	else
		delta = dFdx(IN.texCoord); // Assuming texture width is 512

	for(int i=0; i<7; i++)
	{
		vec2 offset = delta*(i-3);
		vec4 tmp = textureLod(sceneTex, IN.texCoord.xy + offset, time*2);
		computed += tmp * scaleFactors[i];

	}
	return computed;
}


void main(){
//Use texture lod
	//vec4 sceneColor = textureLod(sceneTex, IN.texCoord, time);
	fragColour = blur(sceneTex, IN.texCoord, time);
}