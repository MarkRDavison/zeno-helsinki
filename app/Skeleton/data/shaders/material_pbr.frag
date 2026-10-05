#version 450

struct Material {
    vec4 color;
};

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(std430, binding = 1) readonly buffer MaterialBufferObject {
    Material materials[];
};

layout(binding = 2) uniform sampler2D texSampler;

layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in flat int fragMaterialIndex;
layout(location = 3) in vec3 fragNormal;
layout(location = 4) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

// World-space sun: direction from the surface toward the light. Slice 2.3 moves this to a buffer.
const vec3 kSunDirection = normalize(vec3(0.45, 0.85, 0.30));
const vec3 kSunColor = vec3(1.0, 0.97, 0.90);
const vec3 kAmbient = vec3(0.18);
// Low-poly faces share one normal; a tight lobe (32+) is a few pixels and easy to miss.
const float kShininess = 8.0;
const float kSpecularStrength = 0.35;

void main()
{
    vec3 albedo = texture(texSampler, fragTexCoord).rgb * materials[fragMaterialIndex].color.xyz;
    vec3 n = normalize(fragNormal);
    float ndotl = max(dot(n, kSunDirection), 0.0);

    vec3 cameraPos = inverse(ubo.view)[3].xyz;
    vec3 viewDir = normalize(cameraPos - fragWorldPos);
    vec3 halfDir = normalize(kSunDirection + viewDir);
    float spec = pow(max(dot(n, halfDir), 0.0), kShininess);
    spec *= step(0.0, ndotl);

    vec3 lit = albedo * (kAmbient + kSunColor * ndotl) + vec3(1.0) * spec * kSpecularStrength;
    outColor = vec4(lit, 1.0);
}
