#version 450

layout(binding = 0) uniform sampler2D inputColor;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main() {
    // Simple pass-through. Replace or extend this with your post-processing.
    outColor = texture(inputColor, fragUV);
}