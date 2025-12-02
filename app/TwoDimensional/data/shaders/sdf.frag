#version 450

layout(binding = 1) uniform sampler2D atlas;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

void main()
{
    vec4 base_color = vec4(1.0, 1.0, 1.0, 1.0);
    float dist = texture(atlas, fragTexCoord).r;
    dist -= 0.15;
    float smoothing = fwidth(dist);
    float alpha = clamp(dist / smoothing, 0.0, 1.0);
    outColor = vec4(base_color.rgb, alpha);
}