#version 450

#extension GL_KHR_vulkan_glsl : enable

layout(push_constant) uniform PushConstants {
    vec2 origin;
    float tileSize;
    int cameraIndex;
    vec2 aabbMin;
    vec2 aabbMax;
} pc;

layout(binding = 0) uniform CameraBuffer
{
    mat4 view;
    mat4 proj;
} ubo[4];

layout(location = 0) out vec2 worldXY;
layout(location = 1) out vec2 fillOrigin;
layout(location = 2) out float fillTileSize;

const vec2 corners[4] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0));

int cornerForVertex(uint v)
{
    const int map[6] = int[6](0, 1, 2, 2, 1, 3);
    return map[v];
}

void main()
{
    int ci = cornerForVertex(uint(gl_VertexIndex));
    vec2 local = corners[ci];
    worldXY = mix(pc.aabbMin, pc.aabbMax, local);
    fillOrigin = pc.origin;
    fillTileSize = pc.tileSize;

    gl_Position = ubo[pc.cameraIndex].proj * ubo[pc.cameraIndex].view * vec4(worldXY, 0.0, 1.0);
}
