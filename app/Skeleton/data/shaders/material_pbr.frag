#version 450

struct Material {
    vec4 color;
};

layout(std430, binding = 1) readonly buffer MaterialBufferObject {
    Material materials[];
};

layout(binding = 2) uniform sampler2D texSampler;

layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in flat int fragMaterialIndex;
layout(location = 3) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

// World-space sun: direction from the surface toward the light. Slice 2.3 moves this to a buffer.
const vec3 kSunDirection = normalize(vec3(0.45, 0.85, 0.30));
const vec3 kSunColor = vec3(1.0, 0.97, 0.90);
const vec3 kAmbient = vec3(0.18);

void main()
{
    vec3 albedo = texture(texSampler, fragTexCoord).rgb * materials[fragMaterialIndex].color.xyz;
    vec3 n = normalize(fragNormal);
    float ndotl = max(dot(n, kSunDirection), 0.0);
    vec3 lit = albedo * (kAmbient + kSunColor * ndotl);
    outColor = vec4(lit, 1.0);
}
