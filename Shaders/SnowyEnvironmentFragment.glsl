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

out vec4 fragColour[3];

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
amount *= smoothstep(0.0, 1.0, IN.normal.y); //Snooth out the transition. If less than or equal to 0, return 0. If greater than or equal to 1, return 1. 
//Else return a smooth interpolation between 0 and 1.


vec4 snow = vec4(1.0);
vec4 texIn = texture(diffuseTex, IN.texCoord);
vec4  finalColour = mix(texIn, snow, amount);
finalColour.a = texIn.a;
fragColour[0] = finalColour;
//fragColour[0] = texture(diffuseTex, IN.texCoord);
fragColour[1] = vec4(normal * 0.5 + 0.5, 1.0);

float specular = mix(0.1, 1.0, amount); // snow is shiny
float shininess = mix(16.0, 128.0, amount);
fragColour[2] = vec4(specular, shininess, amount, 1.0);  //Output material properties. R is Specular. G is shininess. B is snow coverage. A is albedo override


}