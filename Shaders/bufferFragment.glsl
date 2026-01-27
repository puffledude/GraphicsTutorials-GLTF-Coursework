#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D normalTex;

uniform mat4 viewMatrix;
in Vertex{

	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;  //Fragpos
}IN;



out vec4 fragColour[4];

void main(void){
mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

vec3 normal = texture(normalTex, IN.texCoord).rgb * 2.0 - 1.0;
normal = normalize(TBN * normalize(normal));

if (texture(diffuseTex, IN.texCoord).a < 0.1){
	discard;
}

fragColour[0] = texture(diffuseTex, IN.texCoord);  
fragColour[1] = vec4(normal * 0.5 + 0.5, 1.0);  //Outputted normals
fragColour[2] = vec4(0.1, 16.0, 0.0, 1.0);  //Default values
vec3 viewPos = (viewMatrix * vec4(IN.worldPos, 1.0)).xyz;
fragColour[3] = vec4(viewPos, 1.0);

}