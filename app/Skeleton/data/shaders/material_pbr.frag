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
} ubo;

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

layout(std140, binding = 4) uniform PointLights {
    ivec4 count;
    vec4 positionRadius[4];
    vec4 colorIntensity[4];
} points;

layout(binding = 5) uniform sampler2DShadow shadowMap;

layout(std140, binding = 6) uniform ShadowBuffer {
    mat4 view;
    mat4 proj;
} shadowCam;

layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in flat int fragMaterialIndex;
layout(location = 3) in vec3 fragNormal;
layout(location = 4) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

float interleavedGradientNoise(vec2 p)
{
    return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715))));
}

vec2 vogelDisk(int i, int n, float theta)
{
    const float goldenAngle = 2.39996323;
    float r = sqrt((float(i) + 0.5) / float(n));
    float a = float(i) * goldenAngle + theta;
    return r * vec2(cos(a), sin(a));
}

float sunShadow(vec3 worldPos, vec3 n, vec3 sunDir)
{
    vec4 lightClip = shadowCam.proj * shadowCam.view * vec4(worldPos, 1.0);
    if (lightClip.w <= 0.0)
    {
        return 1.0;
    }

    vec3 ndc = lightClip.xyz / lightClip.w;
    vec2 uv = ndc.xy * 0.5 + 0.5;
    float ref = ndc.z;
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0 || ref < 0.0 || ref > 1.0)
    {
        return 1.0;
    }

    float bias = max(0.004 * (1.0 - max(dot(n, sunDir), 0.0)), 0.0008);
    float z = ref - bias;
    vec2 texel = 1.0 / vec2(textureSize(shadowMap, 0));
    float theta = interleavedGradientNoise(gl_FragCoord.xy) * 6.2831853;
    const int taps = 16;
    const float radius = 3.5;
    float lit = 0.0;
    for (int i = 0; i < taps; ++i)
    {
        lit += texture(shadowMap, vec3(uv + vogelDisk(i, taps, theta) * radius * texel, z));
    }
    return lit / float(taps);
}

void main()
{
    Material mat = materials[fragMaterialIndex];
    vec3 albedo = texture(texSamplers[nonuniformEXT(mat.albedoIndex)], fragTexCoord).rgb * mat.color.xyz;
    vec3 n = normalize(fragNormal);
    vec3 sunDir = normalize(sun.direction);
    float ndotl = max(dot(n, sunDir), 0.0);
    float shadow = sunShadow(fragWorldPos, n, sunDir);

    vec3 cameraPos = inverse(ubo.view)[3].xyz;
    vec3 viewDir = normalize(cameraPos - fragWorldPos);
    vec3 halfDir = normalize(sunDir + viewDir);

    float shininess = mat.specular.a;
    float spec = 0.0;
    if (shininess > 0.0)
    {
        spec = pow(max(dot(n, halfDir), 0.0), shininess);
        spec *= step(0.0, ndotl);
    }

    vec3 diffuse = sun.ambient + sun.color * sun.intensity * ndotl * shadow;

    int nLights = min(points.count.x, 4);
    for (int i = 0; i < nLights; ++i)
    {
        vec4 posR = points.positionRadius[i];
        vec4 colI = points.colorIntensity[i];
        float radius = posR.w;
        float intensity = colI.w;
        if (radius <= 0.0 || intensity <= 0.0)
        {
            continue;
        }

        vec3 toLight = posR.xyz - fragWorldPos;
        float d = length(toLight);
        if (d >= radius || d < 1e-4)
        {
            continue;
        }

        vec3 lDir = toLight / d;
        float ndotlP = max(dot(n, lDir), 0.0);
        float falloff = 1.0 - d / radius;
        float atten = intensity * falloff * falloff;
        diffuse += colI.rgb * ndotlP * atten;
    }

    vec3 lit = albedo * diffuse + mat.specular.rgb * spec * sun.specularStrength * shadow;
    outColor = vec4(lit, 1.0);
}
