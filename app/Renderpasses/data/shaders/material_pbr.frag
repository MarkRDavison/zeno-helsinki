#version 450

layout(binding = 1) uniform MaterialBufferObject {
    vec3 color;
    float pad;
} material;
layout(binding = 2) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexCoord) * vec4(material.color, 1.0);
}