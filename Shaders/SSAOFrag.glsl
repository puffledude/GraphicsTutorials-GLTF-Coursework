#version 330 core

uniform sampler2D gPosTex; //Rendered Gbuffer
uniform sampler2D normalTex;  //Normal Gbuffer
uniform sampler2D texNoise;


uniform vec3 samples[64];
uniform mat4 projMatrix;

uniform float height;
uniform float width;

in Vertex{
vec2 texCoord;
} IN;


out float fragColour;

vec2 noiseScale = vec2(width/4.0f, height/4.0f);
int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

void main()
{
vec3 fragPos = texture(gPosTex, IN.texCoord).xyz;

vec3 normal = texture(normalTex, IN.texCoord).xyz;
normal = normalize(normal * 2.0 - 1.0);

vec3 randomVec = texture(texNoise, IN.texCoord * noiseScale).xyz * 2.0 - 1.0;

vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
vec3 bitangent = cross(normal, tangent);
mat3 TBN = mat3(tangent, bitangent, normal);

float occlusion = 0.0;

for (int i = 0; i < kernelSize; ++i)
{
    vec3 samplePos = TBN * samples[i];
    samplePos = fragPos + samplePos * radius;

    vec4 offset = vec4(samplePos, 1.0);
    offset = projMatrix * offset;
    offset.xyz /= offset.w;
    offset.xyz = offset.xyz * 0.5 + 0.5;

    float sampleDepth = texture(gPosTex, offset.xy).z;
    float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));

    occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
}

occlusion = 1.0 - (occlusion / kernelSize);
fragColour = occlusion;
//vec3 p = texture(gPosTex, IN.texCoord).xyz;
//fragColour = (p.z + 50.0) / 100.0;   // normalize manually for viewing
}
