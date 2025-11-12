#version 450

#extension GL_KHR_vulkan_glsl : enable

layout(set = 0, binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) out vec3 outUVW;

// Hardcoded cube vertices (36 for 12 triangles)
vec3 skyboxVertices[36] = vec3[](
    // +X
    vec3(1.0,  1.0, -1.0), vec3(1.0, -1.0, -1.0), vec3(1.0, -1.0,  1.0),
    vec3(1.0, -1.0,  1.0), vec3(1.0,  1.0,  1.0), vec3(1.0,  1.0, -1.0),
    // -X
    vec3(-1.0,  1.0,  1.0), vec3(-1.0, -1.0,  1.0), vec3(-1.0, -1.0, -1.0),
    vec3(-1.0, -1.0, -1.0), vec3(-1.0,  1.0, -1.0), vec3(-1.0,  1.0,  1.0),
    // +Y
    vec3(-1.0, 1.0, -1.0), vec3(-1.0, 1.0,  1.0), vec3( 1.0, 1.0,  1.0),
    vec3( 1.0, 1.0,  1.0), vec3( 1.0, 1.0, -1.0), vec3(-1.0, 1.0, -1.0),
    // -Y
    vec3(-1.0, -1.0,  1.0), vec3(-1.0, -1.0, -1.0), vec3( 1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0), vec3( 1.0, -1.0,  1.0), vec3(-1.0, -1.0,  1.0),
    // +Z
    vec3(-1.0,  1.0, 1.0), vec3(-1.0, -1.0, 1.0), vec3( 1.0, -1.0, 1.0),
    vec3( 1.0, -1.0, 1.0), vec3( 1.0,  1.0, 1.0), vec3(-1.0,  1.0, 1.0),
    // -Z
    vec3( 1.0,  1.0, -1.0), vec3( 1.0, -1.0, -1.0), vec3(-1.0, -1.0, -1.0),
    vec3(-1.0, -1.0, -1.0), vec3(-1.0,  1.0, -1.0), vec3( 1.0,  1.0, -1.0)
);

void main() {
    vec3 pos = skyboxVertices[gl_VertexIndex];
    outUVW = pos;

    // Remove translation from the view matrix
    mat4 rotView = mat4(mat3(ubo.view));
    gl_Position = ubo.proj * rotView * vec4(pos, 1.0);
}