#pragma once

#include <string>

namespace hl
{

	class WellKnownResources
	{
		WellKnownResources() = delete;
	public:

		static constexpr const char TextVertexShader_Name[] = "HELSINKI_TEXT_VERTEX_SHADER";
		static constexpr const char SdfFragmentShader_Name[] = "HELSINKI_SDF_FRAGMENT_SHADER";
		static constexpr const char TextFragmentShader_Name[] = "HELSINKI_TEXT_FRAGMENT_SHADER";
		static constexpr const char FullscreenSampleVertexShader_Name[] = "HELSINKI_FULLSCREEN_SAMPLE_VERTEX_SHADER";
		static constexpr const char CompositeFragmentShader_Name[] = "HELSINKI_COMPOSITE_FRAGMENT_SHADER";



		static constexpr const char TextVertexShader_Content[] = R"""(
#version 450

#extension GL_KHR_vulkan_glsl : enable

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 colour;
    int atlasIndex;
    int pad[3];
} pc; 

layout(binding = 0) uniform UniformBufferObject { 
    mat4 view; 
    mat4 proj; 
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out flat int atlasIndex;
layout(location = 2) out vec4 textColor;

void main()
{
    gl_Position = ubo.proj * ubo.view * pc.model * vec4(inPosition, 0.0, 1.0);
    fragTexCoord = inTexCoord;
    atlasIndex = pc.atlasIndex;
    textColor = pc.colour;
}
)""";

		static constexpr const char SdfFragmentShader_Content[] = R"""(
#version 450

layout(binding = 1) uniform sampler2D atlas[64];

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in flat int atlasIndex;
layout(location = 2) in vec4 textColor;
layout(location = 0) out vec4 outColor;

void main()
{
    vec4 base_color = textColor;
    float dist = texture(atlas[atlasIndex], fragTexCoord).r;
    dist -= 0.15;
    float smoothing = fwidth(dist);
    float alpha = clamp(dist / smoothing, 0.0, 1.0);
    outColor = vec4(base_color.rgb, alpha);
}
)""";

		static constexpr const char TextFragmentShader_Content[] = R"""(
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

)""";

		static constexpr const char FullscreenSampleVertexShader_Content[] = R"""(
#version 450

#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) out vec2 fragUV;

// Full-screen triangle (covers NDC [-1,1] x [-1,1])
const vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0,  3.0),
    vec2( 3.0, -1.0)
);


// UVs chosen so the triangle maps the texture over the screen correctly
const vec2 uvs[3] = vec2[](
    vec2(0.0, 0.0),
    vec2(0.0, 2.0),
    vec2(2.0, 0.0)
);

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragUV = uvs[gl_VertexIndex];
}
)""";

        static constexpr const char CompositeFragmentShader_Content[] = R"""(
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
)""";
		
		static bool IsWellKnown(const std::string& text)
		{
			if (text == TextVertexShader_Name ||
				text == SdfFragmentShader_Name ||
				text == TextFragmentShader_Name ||
				text == FullscreenSampleVertexShader_Name ||
				text == CompositeFragmentShader_Name)
			{
				return true;
			}

			return false;
		}

		static std::string GetWellKnown(const std::string& text)
		{
			if (text == TextVertexShader_Name)
			{
				return TextVertexShader_Content;
			}
			else if (text == SdfFragmentShader_Name)
			{
				return SdfFragmentShader_Content;
			}
			else if (text == TextFragmentShader_Name)
			{
				return TextFragmentShader_Content;
			}
			else if (text == FullscreenSampleVertexShader_Name)
			{
				return FullscreenSampleVertexShader_Content;
			}
			else if (text == CompositeFragmentShader_Name)
			{
				return CompositeFragmentShader_Content;
			}

			throw std::runtime_error("Cannot retrieve well known resource: " + text);
		}

	};

}