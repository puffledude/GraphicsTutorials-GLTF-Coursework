#version 330 core

in vec3 vertexPos;
in vec3 instancePos;
in float size;
in vec4 instanceColour;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

out Vertex{
	vec4 colour;
} OUT;

void main(void){

	OUT.colour = instanceColour;
	
	vec3 scaledPos = vertexPos * size;  //Scales the vertex
	vec4 worldPos = vec4(scaledPos + instancePos, 1.0);  //Translates to world instance position
	
    gl_Position = projMatrix * viewMatrix * modelMatrix * worldPos;
}