#version 330 core

uniform sampler2D sceneTex;
uniform float borderDistance;
uniform int width;
uniform int height;
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

out vec4 fragColor;


vec4 blur(sampler2D sceneTex, vec2 texCoord, float amount)
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
		vec4 tmp = textureLod(sceneTex, IN.texCoord.xy + offset, amount*2);
		computed += tmp * scaleFactors[i];

	}
	return computed;
}


void main(void)
{

vec2 uv = IN.texCoord;
float edgeDist = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));

if (IN.texCoord.x < borderDistance || IN.texCoord.x > width - borderDistance)
{
	fragColor = texture(sceneTex, IN.texCoord);
}
else
{
	float amount=  distance(IN.texCoord, vec2(borderDistance,0)); 
	fragColor = blur(sceneTex, IN.texCoord, amount);

}}