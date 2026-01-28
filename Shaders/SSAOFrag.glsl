#version 330 core

uniform sampler2D gPosTex;     // View-space position
uniform sampler2D normalTex;   // World-space normals
uniform sampler2D texNoise;

uniform mat4 viewMatrix;
uniform vec3 samples[64];
uniform mat4 projMatrix;

uniform float height;
uniform float width;

in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColour;

const int kernelSize = 64;
const float radius = 0.5;
const float bias   = 0.025;

void main()
{
    vec2 noiseScale = vec2(width / 4.0, height / 4.0);

    // Get view-space position
    vec3 fragPos = texture(gPosTex, IN.texCoord).xyz;
    
    // Get world normal and convert to view space
    vec3 worldNormal = texture(normalTex, IN.texCoord).xyz * 2.0 - 1.0;
    mat3 normalMatrix = transpose(inverse(mat3(viewMatrix)));
    vec3 normal = normalize(normalMatrix * worldNormal);

    // Get random vector
    vec3 randomVec = texture(texNoise, IN.texCoord * noiseScale).xyz * 2.0 - 1.0;

    // Create TBN matrix
    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;

    for (int i = 0; i < kernelSize; ++i)
    {
        // Get sample position in view space
        vec3 samplePos = TBN * samples[i];
        samplePos = fragPos + samplePos * radius;

        // Project to screen space
        vec4 offset = projMatrix * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        // Check bounds
        if (offset.x < 0.0 || offset.x > 1.0 ||
            offset.y < 0.0 || offset.y > 1.0)
        {
            continue;
        }

        // Get depth at sample point
        float sampleDepth = texture(gPosTex, offset.xy).z;

        // Range check
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));

        // Occlusion test
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / float(kernelSize));
    fragColour = vec4(occlusion, 0.0, 0.0, 1.0);
}