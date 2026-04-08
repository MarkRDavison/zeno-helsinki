#version 450

layout(binding = 1) uniform sampler2D atlas[64];

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in flat int atlasIndex;
layout(location = 2) in vec4 textColor;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor.rgb = textColor.rgb;
    outColor.a = texture(atlas[atlasIndex], fragTexCoord).r;
}
