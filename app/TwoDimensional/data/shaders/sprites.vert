#version 450

#extension GL_KHR_vulkan_glsl : enable

layout(push_constant) uniform PushConstants {
    mat4 model;
} pc;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

// layout(location = 0) in vec3 inPosition;
// layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

// 4 corners for 2-triangle quad
// index: 0 1 2 3 -> (-1,-1), (1,-1), (-1,1), (1,1)
const vec2 corners[4] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0)
);

void main() {
    vec2 local = corners[gl_VertexIndex];
    
    vec2 pos = (local * 1.0f);

    gl_Position = ubo.proj * ubo.view * pc.model * vec4(pos, 0.0, 1.0);
    fragTexCoord = corners[gl_VertexIndex];
}