#version 450

layout(binding = 1) readonly buffer SpriteFrameSSBO {
    vec4 frames[];
};

layout(binding = 2) uniform sampler2D texSampler;

layout(location = 0) in vec2 worldXY;
layout(location = 1) in vec2 fillOrigin;
layout(location = 2) in float fillTileSize;

layout(location = 0) out vec4 outColor;

void main()
{
    vec2 local = fract((worldXY - fillOrigin) / fillTileSize);
    vec4 uvRect = frames[0];
    vec2 uv = mix(uvRect.xy, uvRect.zw, local);
    outColor = texture(texSampler, uv);
}
