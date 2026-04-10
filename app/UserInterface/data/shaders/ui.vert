#version 450 

#extension GL_KHR_vulkan_glsl : enable

layout(binding = 0) uniform UniformBufferObject { 
    mat4 view; 
    mat4 proj; 
} ubo; 

layout(location = 0) in vec2 inPosition;

void main() { 
    gl_Position = ubo.proj * ubo.view * vec4(inPosition.x, inPosition.y, 0.0, 1.0); 
}