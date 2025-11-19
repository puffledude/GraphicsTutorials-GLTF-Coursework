#version 330 core

uniform sampler2D sceneTex;



in Vertex{
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main(){
	vec4 sceneColor = texture(sceneTex, IN.texCoord);
	fragColour = sceneColor;
}