#version 450

#extension GL_EXT_nonuniform_qualifier : require

// Keep in sync with MAX_MATERIAL_TEXTURES in RendererConfiguration.hpp
#define MAX_MATERIAL_TEXTURES 64

struct Material {
    vec4 color;
    vec4 specular;
    uint albedoIndex;
    uint pad[3];
};

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo[4];

layout(std430, binding = 1) readonly buffer MaterialBufferObject {
    Material materials[];
};

layout(binding = 2) uniform sampler2D texSamplers[MAX_MATERIAL_TEXTURES];

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
layout(location = 5) in flat int fragCameraIndex;
layout(location = 6) in flat int fragSkipShadow;

layout(location = 0) out vec4 outColor;

const float GhostAlpha = 0.45;

void main()
{
    Material mat = materials[fragMaterialIndex];
    vec3 albedo = texture(texSamplers[nonuniformEXT(mat.albedoIndex)], fragTexCoord).rgb * mat.color.xyz;
    vec3 n = normalize(fragNormal);
    vec3 sunDir = normalize(sun.direction);
    float ndotl = max(dot(n, sunDir), 0.0);

    vec3 cameraPos = inverse(ubo[fragCameraIndex].view)[3].xyz;
    vec3 viewDir = normalize(cameraPos - fragWorldPos);
    vec3 halfDir = normalize(sunDir + viewDir);

    float shininess = mat.specular.a;
    float spec = 0.0;
    if (shininess > 0.0)
    {
        spec = pow(max(dot(n, halfDir), 0.0), shininess);
        spec *= step(0.0, ndotl);
    }

    vec3 diffuse = sun.ambient + sun.color * sun.intensity * ndotl;
    vec3 lit = albedo * diffuse + mat.specular.rgb * spec * sun.specularStrength;
    outColor = vec4(lit, GhostAlpha + 0.0 * float(fragSkipShadow));
}
