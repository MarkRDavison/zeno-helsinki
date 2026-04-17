#version 450 

#extension GL_KHR_vulkan_glsl : enable

layout(binding = 0) uniform UniformBufferObject { 
    mat4 view; 
    mat4 proj; 
} ubo; 

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexUv;
layout(location = 2) in vec3 inColor;
layout(location = 3) in uint inRenderType;

layout(location = 0) out vec3 fragColor; 
layout(location = 1) out vec2 texUv; 
layout(location = 2) out flat uint textureIndex;
layout(location = 3) out flat uint renderType;

void main() { 
    gl_Position = ubo.proj * ubo.view * vec4(inPosition.x, inPosition.y, 0.0, 1.0); 
    fragColor = inColor;
    texUv = inTexUv;
    textureIndex = inRenderType;
    renderType = inRenderType;
}