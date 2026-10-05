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

layout(std140, binding = 3) uniform SunBuffer {
    vec3 direction;
    float intensity;
    vec3 color;
    float specularStrength;
    vec3 ambient;
    float _pad;
} sun;

layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in flat int fragMaterialIndex;
layout(location = 3) in vec3 fragNormal;
layout(location = 4) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

// Low-poly faces share one normal; a tight lobe (32+) is a few pixels and easy to miss.
const float kShininess = 8.0;

void main()
{
    vec3 albedo = texture(texSampler, fragTexCoord).rgb * materials[fragMaterialIndex].color.xyz;
    vec3 n = normalize(fragNormal);
    vec3 sunDir = normalize(sun.direction);
    float ndotl = max(dot(n, sunDir), 0.0);

    vec3 cameraPos = inverse(ubo.view)[3].xyz;
    vec3 viewDir = normalize(cameraPos - fragWorldPos);
    vec3 halfDir = normalize(sunDir + viewDir);
    float spec = pow(max(dot(n, halfDir), 0.0), kShininess);
    spec *= step(0.0, ndotl);

    vec3 lit = albedo * (sun.ambient + sun.color * sun.intensity * ndotl)
        + vec3(1.0) * spec * sun.specularStrength;
    outColor = vec4(lit, 1.0);
}
