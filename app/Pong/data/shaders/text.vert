#version 450

#extension GL_KHR_vulkan_glsl : enable

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 colour;
    int atlasIndex;
    int pad[3];
} pc; 

layout(binding = 0) uniform UniformBufferObject { 
    mat4 view; 
    mat4 proj; 
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out flat int atlasIndex;
layout(location = 2) out vec4 textColor;

void main()
{
    gl_Position = ubo.proj * ubo.view * pc.model * vec4(inPosition, 0.0, 1.0);
    fragTexCoord = inTexCoord;
    atlasIndex = pc.atlasIndex;
    textColor = pc.colour;
}