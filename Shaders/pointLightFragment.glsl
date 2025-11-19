#version 330 core

uniform sampler2D depthTex;
uniform sampler2D normalTex;
uniform sampler2D materialTex;
uniform sampler2D shadowTex;

uniform vec3 cameraPos;
uniform vec2 pixelSize;

uniform vec3 lightPos;
uniform vec4 lightColour;
uniform float lightRadius;
uniform mat4 invProjViewMatrix;
uniform mat4 shadowMatrix;

out vec4 diffuseOutput;
out vec4 specularOutput;

void main(void){
float shadowBias = 0.005;

vec2 texCoord = vec2(gl_FragCoord.xy * pixelSize);
float depth = texture(depthTex, texCoord.xy).r;

vec3 ndcPos = vec3(texCoord, depth) * 2.0 - 1.0;
vec4 invClipPos = invProjViewMatrix * vec4(ndcPos, 1.0);
vec3 worldPos = invClipPos.xyz / invClipPos.w;

float dist = length(lightPos - worldPos);
float atten = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);

if(atten <= 0.0){
	discard;
}

vec3 normal = normalize(texture(normalTex, texCoord.xy).xyz * 2.0 - 1.0);
diffuseOutput = vec4(normal, 1.0);
vec3 incident = normalize(lightPos - worldPos);
vec3 viewDir = normalize(cameraPos - worldPos);
vec3 halfDir = normalize(incident + viewDir);

float lambert = clamp(dot(incident, normal), 0.0, 1.0);

float specular = texture(materialTex, texCoord.xy).r;
float shininess = texture(materialTex, texCoord.xy).g;

float rFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
float specFactor = pow(max(dot(normal, halfDir), 0.0), shininess) * specular;
//float specFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
//specFactor = pow(specFactor, 60.0);
float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 5.0);
float snowReflectivity =1;  // tweak this value
float fresnelSpecular = snowReflectivity* fresnel;

float up = clamp(dot(normal, vec3(0,1,0)), 0.0, 1.0);
float snowMask = pow(up, 3.0);

fresnelSpecular *= snowMask;


specFactor = fresnelSpecular + specFactor;



//For shadowing
vec4 lightSpacePos = shadowMatrix * vec4(worldPos, 1.0);
vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
projCoords = projCoords * 0.5 + 0.5;

float shadowDepth = texture(shadowTex, projCoords.xy).r;
float shadow = projCoords.z - shadowBias > shadowDepth ? 0.0 : 1.0;
float visability = shadow;

vec3 attenuated = lightColour.xyz * atten;

diffuseOutput = vec4(attenuated * lambert * visability, 1.0);
specularOutput = vec4(attenuated * specFactor * visability *0.33, 1.0);


}