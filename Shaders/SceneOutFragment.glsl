#version 330 core


uniform sampler2D sceneTex;
uniform sampler2D depthTex;

in Vertex{
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void)
{
	float sceneDepth = texture(depthTex, IN.texCoord).r;
	if (sceneDepth >= 0.9999)
		discard;
	fragColour = texture(sceneTex, IN.texCoord);
}