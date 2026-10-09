#version 450 

#extension GL_KHR_vulkan_glsl : enable

layout(binding = 0) uniform CameraBuffer
{
    mat4 view;
    mat4 proj;
} ubo[4];

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in float inTexIndex;
layout(location = 4) in float inSdf;

layout(location = 0) out vec4 fragColor; 
layout(location = 1) out vec2 fragTexCoord; 
layout(location = 2) out float fragTexIndex;
layout(location = 3) out float fragSdf;

void main() { 
    gl_Position = ubo[0].proj * ubo[0].view * vec4(inPosition.x, inPosition.y, 0.0, 1.0); 
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragTexIndex = inTexIndex;
    fragSdf = inSdf;
}
