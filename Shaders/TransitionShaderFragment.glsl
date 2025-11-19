#version 330 core

uniform sampler2D sceneTex;
uniform float time;


in Vertex{
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main(){
//Use texture lod
	vec4 sceneColor = textureLod(sceneTex, IN.texCoord, time);
	fragColour = sceneColor;
}