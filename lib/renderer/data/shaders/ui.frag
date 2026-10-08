#version 450

#extension GL_EXT_nonuniform_qualifier : require

#include "ui_constants.glsl"

layout(binding = 1) uniform sampler2D texSamplers[MAX_UI_TEXTURES];

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in float fragTexIndex;

layout(location = 0) out vec4 outFragColor;

void main()
{
    const int texIndex = int(fragTexIndex);
    vec4 tex = texture(texSamplers[nonuniformEXT(texIndex)], fragTexCoord);

    if (texIndex == 1)
    {
        float dist = tex.r - 0.15;
        float smoothing = fwidth(dist);
        float alpha = clamp(dist / smoothing, 0.0, 1.0);
        outFragColor = vec4(fragColor.rgb, alpha * fragColor.a);
    }
    else
    {
        outFragColor = tex * fragColor;
    }
}
