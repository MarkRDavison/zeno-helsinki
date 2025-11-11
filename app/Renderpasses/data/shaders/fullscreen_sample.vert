#version 450

#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) out vec2 fragUV;

// Full-screen triangle (covers NDC [-1,1] x [-1,1])
const vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0,  3.0),
    vec2( 3.0, -1.0)
);


// UVs chosen so the triangle maps the texture over the screen correctly
const vec2 uvs[3] = vec2[](
    vec2(0.0, 0.0),
    vec2(0.0, 2.0),
    vec2(2.0, 0.0)
);

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragUV = uvs[gl_VertexIndex];
}