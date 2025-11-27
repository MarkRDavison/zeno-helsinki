#version 450

layout(binding = 0) uniform sampler2D inputColor;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(texture(inputColor, fragUV).rgb, 1.0);
}