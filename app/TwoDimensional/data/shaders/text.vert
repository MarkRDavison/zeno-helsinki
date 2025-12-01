#version 450

layout(push_constant) uniform PushConstants {
    int cellIndex;
} pc; 

layout(binding = 0) uniform UniformBufferObject { 
    mat4 view; 
    mat4 proj; 
} ubo; 

layout(location = 0) out vec2 fragTexCoord; // normalized atlas UV (0..1)

void main() {

    float size = 256.0f;
    // -------- Quad in local space (64x64 at position 128,128) --------
    const vec2 localPos[6] = vec2[](
        vec2(0.0, 0.0),
        vec2(size, 0.0),
        vec2(size, size),

        vec2(0.0, 0.0),
        vec2(size, size),
        vec2(0.0, size)
    );

    // Add world position offset (128,128)
    vec2 worldPos = localPos[gl_VertexIndex] + vec2(128.0, 128.0);

    // Convert to vec4 for matrix multiply
    vec4 worldPos4 = vec4(worldPos, 0.0, 1.0);

    // Final position
    gl_Position = ubo.proj * ubo.view * worldPos4;


    // ---------------- Atlas UV computation (unchanged) ----------------

    const float uvDelta = 0.001;
    const float cellSizePx  = 64.0;
    const float atlasSizePx = 2048.0;
    const int   cellsPerRow = int(atlasSizePx / cellSizePx); // 32

    const vec2 CELL_UV[6] = vec2[](
        vec2(0.0 + uvDelta, 0.0 + uvDelta),
        vec2(1.0 - uvDelta, 0.0 + uvDelta),
        vec2(1.0 - uvDelta, 1.0 - uvDelta),

        vec2(0.0 + uvDelta, 0.0 + uvDelta),
        vec2(1.0 - uvDelta, 1.0 - uvDelta),
        vec2(0.0 + uvDelta, 1.0 - uvDelta)
    );

    int col = pc.cellIndex % cellsPerRow;
    int row = pc.cellIndex / cellsPerRow;

    vec2 cellOrigin = vec2(float(col), float(row)) * (cellSizePx / atlasSizePx);
    vec2 cellScale  = vec2(cellSizePx / atlasSizePx);

    outAtlasUV = cellOrigin + CELL_UV[gl_VertexIndex] * cellScale;
}