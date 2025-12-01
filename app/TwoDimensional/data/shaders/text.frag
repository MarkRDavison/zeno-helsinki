#version 450

layout(binding = 1) uniform sampler2D msdf;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;


float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    vec4 bgColor = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 fgColor = vec4(0.0, 0.0, 0.0, 1.0);
    vec3 msd = texture(msdf, fragTexCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float pxRange = 128.0 / 64.0 * 2.0;
    float screenPxDistance = pxRange * (sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    outColor = mix(bgColor, fgColor, opacity);
}