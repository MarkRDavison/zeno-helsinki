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

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 matColor = materials[fragMaterialIndex].color.xyz;
    outColor = texture(texSampler, fragTexCoord) * vec4(matColor, 1.0);
}
