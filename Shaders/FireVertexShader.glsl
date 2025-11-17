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
	
	vec4 centreVS = viewMatrix * modelMatrix * vec4(instancePos, 1.0); //Centre of the quad in view space

	vec2 quadOffset = vertexPos.xy * size; //The 2d offset applied to the particles centre to make the quad


	// Optional: attenuate size with distance if you want screen-size stability
	//float dist = max(-centreVS.z, 0.0001); // positive distance in front of camera
	//quadOffset *= 1.0 / dist;

	// Compose final position in view-space (z stays at centerVS.z)
	vec4 posVS = centreVS + vec4(quadOffset, 0.0, 0.0); //Position in viewspace is the centre + offset

	gl_Position = projMatrix * posVS;



	


	//vec3 scaledPos = vertexPos * size;  //Scales the vertex
	//vec4 worldPos = vec4(scaledPos + instancePos, 1.0);  //Translates to world instance position
	
    //gl_Position = projMatrix * viewMatrix * modelMatrix * worldPos;
}