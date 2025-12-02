#version 450

layout(binding = 1) uniform sampler2D atlas;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor.rgb = vec3(1.0, 1.0, 1.0);
    outColor.a = texture(atlas, fragTexCoord).r;
}
