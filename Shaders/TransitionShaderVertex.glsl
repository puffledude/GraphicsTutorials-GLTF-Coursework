#version 330 core

in vec2 texCoord;
in vec3 position;

out Vertex{
	vec2 texCoord;
} OUT;

void main(void){
	OUT.texCoord = texCoord;
	gl_Position = vec4(position, 1.0);
}