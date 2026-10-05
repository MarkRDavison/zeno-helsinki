#version 450

struct Material {
    vec4 color;
    vec4 specular;
};

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo[4];

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
layout(location = 5) in flat int fragCameraIndex;

layout(location = 0) out vec4 outColor;

const float GhostAlpha = 0.45;

void main()
{
    Material mat = materials[fragMaterialIndex];
    vec3 albedo = texture(texSampler, fragTexCoord).rgb * mat.color.xyz;
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
    outColor = vec4(lit, GhostAlpha);
}
