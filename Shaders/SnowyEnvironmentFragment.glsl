#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D normalTex;

in Vertex{

	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
}IN;

out vec4 fragColour[2];

void main(void){
mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

vec3 normal = texture(normalTex, IN.texCoord).rgb * 2.0 - 1.0;
normal = normalize(TBN * normalize(normal));

if (texture(diffuseTex, IN.texCoord).a < 0.1){
	discard;
}
//I want to make it so the closer the normal is to (0,1,0) the more it blends to white

float up = clamp(dot(normalize(IN.normal), vec3(0,1,0)), 0.0, 1.0);
float amount = pow(up, 3.0);

vec4 snow = vec4(1.0);
vec4 texIn = texture(diffuseTex, IN.texCoord);
vec4 output = mix(texIn, snow, amount);
output.a = texIn.a;
fragColour[0] = output;
//fragColour[0] = texture(diffuseTex, IN.texCoord);
fragColour[1] = vec4(normal * 0.5 + 0.5, 1.0);
}