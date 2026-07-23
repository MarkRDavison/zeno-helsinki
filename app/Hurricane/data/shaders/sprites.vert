#version 450 

#extension GL_KHR_vulkan_glsl : enable

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec2 size;
    int frameIndex;
} pc; 

layout(binding = 0) uniform UniformBufferObject { 
    mat4 view; 
    mat4 proj; 
} ubo; 

// SSBO containing all frames in the spritesheet
layout(binding = 1) readonly buffer SpriteFrameSSBO {
    vec4 frames[]; // each vec4 = (u0, v0, u1, v1)
};

layout(location = 0) out vec2 fragTexCoord; 

const vec2 corners[4] = vec2[]( 
    vec2(0.0, 0.0), // bottom-left 
    vec2(1.0, 0.0), // bottom-right 
    vec2(0.0, 1.0), // top-left 
    vec2(1.0, 1.0) // top-right 
); // 6-vertex triangle-list quad 

int cornerForVertex(uint v) { 
    const int map[6] = int[6](0, 1, 2, 2, 1, 3); 
    return map[v];
} 

void main() { 
    uint v = uint(gl_VertexIndex); 

    int ci = cornerForVertex(v); 

    vec2 local = corners[ci]; 

    vec2 pos2D = local * pc.size;

    // Position in model space (Z = 0) 
    vec4 worldPos = pc.model * vec4(pos2D.x, pos2D.y, 0.0, 1.0); 
    
    // Full transform: model * view * projection 
    gl_Position = ubo.proj * ubo.view * worldPos; 
    
    // Compute UVs from SSBO frame
    vec4 uvRect = frames[uint(pc.frameIndex)];
    fragTexCoord = uvRect.xy + corners[ci] * (uvRect.zw - uvRect.xy);
}