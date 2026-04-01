#version 450

layout(binding = 1) uniform sampler2D atlas[64];

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in flat int atlasIndex;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor.rgb = vec3(1.0, 1.0, 1.0);
    outColor.a = texture(atlas[atlasIndex], fragTexCoord).r;
}
