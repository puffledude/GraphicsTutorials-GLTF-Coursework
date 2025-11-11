#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D specularLight;
uniform sampler2D diffuseLight;
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

	vec3 diffuse = texture(diffuseTex, IN.texCoord).xyz;
	vec3 light = texture(diffuseLight, IN.texCoord).xyz;
	vec3 specular = texture(specularLight, IN.texCoord).xyz;

	fragColour.xyz = diffuse*0.1;
	fragColour.xyz += diffuse*light;
	fragColour.xyz += specular;
	fragColour.a = 1.0;

}
