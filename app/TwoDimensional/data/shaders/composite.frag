#version 450

layout(location = 0) in vec2 fragUV;

layout(binding = 0) uniform sampler2D postTex;
layout(binding = 1) uniform sampler2D uiTex;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 post = texture(postTex, fragUV);
    vec4 ui   = texture(uiTex, fragUV);

    outColor = post * (1.0 - ui.a) + ui;
}