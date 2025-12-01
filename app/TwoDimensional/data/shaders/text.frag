#version 450

layout(binding = 1) uniform sampler2D msdf;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    vec4 bgColor = vec4(1.0, 1.0, 1.0, 0.0); // white background
    vec4 fgColor = vec4(0.0, 0.0, 0.0, 1.0); // black glyph

    vec3 msd = texture(msdf, fragTexCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);

    float scale = 256.0 / 64.0;      // quad size / cell size
    float thickness = 0.5;           // adjust this to make glyph thicker or thinner

    float screenPxDistance = (sd - 0.5) * scale;
    float opacity = screenPxDistance > -thickness ? 1.0 : 0.0;

    outColor = mix(bgColor, fgColor, opacity);
}
