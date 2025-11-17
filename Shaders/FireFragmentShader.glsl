#version 330 core

uniform sampler2D diffuseTex;

in Vertex{
	vec4 colour;
	vec3 worldPos;
}IN;

out vec4 fragColour;
void main(void){
	fragColour = IN.colour;
}