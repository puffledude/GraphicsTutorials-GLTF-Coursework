#version 400 core

uniform sampler2D diffuseTex;
uniform samplerCube cubeTex;

uniform vec3 cameraPos;
in Vertex{
	vec3 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 worldPos;
} IN;

out vec4 fragColour[3];

void main(void)
{
	vec4 diffuse = texture(diffuseTex, IN.texCoord);
	vec3 viewDir = normalize(cameraPos - IN.worldPos);

	vec3 reflectDir = reflect(-viewDir, normalize(IN.normal));
	vec4 reflectTex = texture(cubeTex, reflectDir);

	fragColour[0] = reflectTex +(diffuse * 0.5);
	fragColour[0] += 0.3;
	fragColour[0].a = 1.0;
	fragColour[1] = vec4(0,1,0,1);
	fragColour[2] = vec4(0.5, 64.0, 300.0, 1.0); //Output material properties. R is Specular. G is shininess. B is snow coverage. A is albedo override
}