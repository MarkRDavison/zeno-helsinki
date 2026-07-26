#version 450

#extension GL_KHR_vulkan_glsl : enable

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 color;
} pc; 


struct Camera
{
    mat4 view;
    mat4 proj;
};

layout(binding = 0) uniform CameraBuffer
{
    Camera cameras[4];
} ubo;

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec4 fragColor;

void main()
{
    gl_Position = ubo.cameras[0].proj * ubo.cameras[0].view * pc.model * vec4(inPosition, 0.0, 1.0);
    fragColor = pc.color;
}