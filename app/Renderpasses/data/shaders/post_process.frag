#version 450

layout(binding = 0) uniform sampler2D inputColor;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

#define TYPE 0

void main()
{
    if (TYPE == 1) // Invert Colors
    {
        vec4 c = texture(inputColor, fragUV);
        outColor = vec4(1.0 - c.rgb, c.a);
    }
    else if (TYPE == 2) // Grayscale
    {
        vec4 c = texture(inputColor, fragUV);
        float g = dot(c.rgb, vec3(0.299, 0.587, 0.114));
        outColor = vec4(vec3(g), c.a);
    }
    else if (TYPE == 3) // Slight Contrast Boost
    {
        vec4 c = texture(inputColor, fragUV);
        float contrast = 1.2; 
        outColor = vec4(((c.rgb - 0.5) * contrast + 0.5), c.a);
    }
    else if (TYPE == 4) // Vignette
    {
        vec4 c = texture(inputColor, fragUV);

        float dist = distance(fragUV, vec2(0.5));
        float vignette = smoothstep(0.8, 0.4, dist);

        outColor = vec4(c.rgb * vignette, c.a);
    }
    else if (TYPE == 5) // Cheap Chromatic Aberration
    {
        float off = 0.002;

        float r = texture(inputColor, fragUV + vec2(off, 0.0)).r;
        float g = texture(inputColor, fragUV).g;
        float b = texture(inputColor, fragUV - vec2(off, 0.0)).b;

        outColor = vec4(r, g, b, 1.0);
    }
    else if (TYPE == 6) // Film Grain
    {
        vec4 c = texture(inputColor, fragUV);

        float noise = fract(sin(dot(fragUV, vec2(12.9898, 78.233))) * 43758.5453);
        float strength = 0.05;

        outColor = vec4(c.rgb + noise * strength, c.a);
    }
    else if (TYPE == 7) // Cheap blur
    {
        vec2 texel = 1.0 / vec2(textureSize(inputColor, 0));

        float kernel[5][5] = float[5][5](
            float[5](1, 4, 6, 4, 1),
            float[5](4,16,24,16, 4),
            float[5](6,24,36,24, 6),
            float[5](4,16,24,16, 4),
            float[5](1, 4, 6, 4, 1)
        );

        vec3 sum = vec3(0.0);
        float wsum = 0.0;

        for (int i = -2; i <= 2; i++) {
            for (int j = -2; j <= 2; j++) {
                float w = kernel[i+2][j+2];
                sum += texture(inputColor, fragUV + vec2(i, j) * texel).rgb * w;
                wsum += w;
            }
        }

        outColor = vec4(sum / wsum, 1.0);
    }
    else if (TYPE == 8) // Heat Distortion / Wavy Refraction
    {
        vec2 texel = 1.0 / vec2(textureSize(inputColor, 0));

        float t = sin(dot(fragUV, vec2(12.93, 48.21))) * 5.0;
        float wave = sin(fragUV.y * 40.0 + t) * 0.01;

        vec2 uv = fragUV + vec2(wave, 0.0);

        outColor = texture(inputColor, uv);
    }
    else if (TYPE == 9) // Pixelation + Sharpen
    {
        vec2 res = vec2(160.0, 90.0); // fake low resolution
        vec2 uv = floor(fragUV * res) / res;

        vec4 col = texture(inputColor, uv);

        // simple sharpen
        vec3 c = col.rgb;
        vec3 s = c * 1.4 - 0.2;

        outColor = vec4(s, 1.0);
    }
    else if (TYPE == 10) // Polar Twist
    {
        vec2 c = fragUV - 0.5;
        float r = length(c);
        float a = atan(c.y, c.x);

        // Twist amount depends on radius
        float twist = r * 3.0;

        a += twist;

        vec2 uv = vec2(cos(a), sin(a)) * r + 0.5;

        outColor = texture(inputColor, uv);
    }
    else
    {
        outColor = texture(inputColor, fragUV);
    }
}