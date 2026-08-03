#version 450 

#extension GL_KHR_vulkan_glsl : enable

layout(binding = 0) uniform CameraBuffer
{
    mat4 view;
    mat4 proj;
} ubo[4];

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor; 

void main() { 
    gl_Position = ubo[0].proj * ubo[0].view * vec4(inPosition.x, inPosition.y, 0.0, 1.0); 
    fragColor = inColor;
}