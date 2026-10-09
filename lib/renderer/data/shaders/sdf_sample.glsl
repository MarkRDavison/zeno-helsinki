#ifndef HELSINKI_SDF_SAMPLE_GLSL
#define HELSINKI_SDF_SAMPLE_GLSL

// FreeType default SDF spread (pixels). Slice C may change the bake.
const float SDF_PX_RANGE = 8.0;

float sdfAlpha(float d, vec2 uv, ivec2 atlasSize)
{
    float sd = d - 0.5;
    vec2 unitRange = vec2(SDF_PX_RANGE) / vec2(atlasSize);
    vec2 screenTexSize = vec2(1.0) / fwidth(uv);
    float screenPxRange = max(0.5, dot(unitRange, screenTexSize));
    return clamp(sd * screenPxRange + 0.5, 0.0, 1.0);
}

#endif
