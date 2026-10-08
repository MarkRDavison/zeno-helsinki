#include "Ui/UiLayoutAdapters.hpp"
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Engine/Input/InputManager.hpp>
#include <helsinki/Engine/Ui/UiBatch.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <helsinki/Renderer/Resource/MaterialSystem.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/Resource/SignedDistanceFieldFontResource.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Renderer/RendererShaderRoot.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <GLFW/glfw3.h>
#include <limits>
#include <string>

namespace tower
{
	FontTypeface::FontTypeface(hl::FontResource* font) : _font(font)
	{
	}

	glm::vec2 FontTypeface::layoutText(
		std::string_view text,
		unsigned fontSize,
		std::vector<hl::ui::GlyphVertex>& out) const
	{
		out.clear();
		if (_font == nullptr)
		{
			return { 0.0f, 0.0f };
		}

		const auto generated = _font->generateTextVertexes(std::string(text), fontSize);
		if (generated.empty())
		{
			return { 0.0f, 0.0f };
		}

		glm::vec2 minPos{ std::numeric_limits<float>::max() };
		glm::vec2 maxPos{ std::numeric_limits<float>::lowest() };
		for (const auto& v : generated)
		{
			minPos = glm::min(minPos, v.pos);
			maxPos = glm::max(maxPos, v.pos);
		}

		out.reserve(generated.size());
		for (const auto& v : generated)
		{
			out.push_back(hl::ui::GlyphVertex{ .pos = v.pos - minPos, .uv = v.texCoord });
		}

		return maxPos - minPos;
	}

	UiBatchPaint::UiBatchPaint(hl::UiBatch& batch) : _batch(&batch)
	{
	}

	void UiBatchPaint::fill(const hl::ui::Box& box, glm::vec4 color)
	{
		const float pixel = 6.0f;
		const float TEX_SIZE = 1024.0f;
		const auto texCoords = glm::vec4{ pixel, pixel, 1, 1 } / TEX_SIZE;
		_batch->addQuad(box, color, texCoords, static_cast<float>(UI_TEX_WHITE));
	}

	void UiBatchPaint::sprite(const hl::ui::Box& box, glm::vec4 /*uvRect*/, glm::vec3 color)
	{
		fill(box, glm::vec4{ color, 1.0f });
	}

	void UiBatchPaint::glyphs(
		const std::vector<hl::ui::GlyphVertex>& verts,
		glm::vec2 origin,
		glm::vec3 color)
	{
		std::vector<hl::Vertex22D> converted;
		converted.reserve(verts.size());
		for (const auto& v : verts)
		{
			converted.push_back(hl::Vertex22D{ .pos = v.pos, .texCoord = v.uv });
		}

		_batch->addGlyphs(converted, origin, color, static_cast<float>(UI_TEX_ROBOTO));
	}

	hl::RenderpassInfo makeMenuUiRenderpass(
		const std::string& cameraMatrixResourceId,
		const std::string& sheetName)
	{
		return hl::RenderpassInfo
		{
			.name = "ui_renderpass",
			.inputs = {},
			.outputs =
			{
				hl::ResourceInfo
				{
					.name = "ui_color",
					.type = hl::ResourceType::Color,
					.format = "VK_FORMAT_B8G8R8A8_SRGB",
					.clear = VkClearValue{ .color = { 0.0f, 0.0f, 0.0f, 1.0f} }
				}
			},
			.pipelineGroups =
			{
				{
					hl::PipelineInfo
					{
						.name = "ui_pipeline",
						.shaderVert = std::string(hl::RendererShaderRoot) + "/ui.vert",
						.shaderFrag = std::string(hl::RendererShaderRoot) + "/ui.frag",
						.descriptorSets =
						{
							hl::DescriptorSetInfo
							{
								.bindings =
								{
									hl::DescriptorBinding
									{
										.binding = 0,
										.type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
										.stage = "VERTEX",
										.resource = cameraMatrixResourceId,
										.count = MAX_CAMERAS,
										.updateFrequency = hl::DescriptorUpdateFrequency::Static
									},
									hl::DescriptorBinding
									{
										.binding = 1,
										.type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
										.stage = "FRAGMENT",
										.resource = sheetName,
										.count = static_cast<uint32_t>(MAX_UI_TEXTURES),
										.updateFrequency = hl::DescriptorUpdateFrequency::Static,
										.partiallyBound = true,
										.updateAfterBind = true
									}
								}
							}
						},
						.vertexInputInfo = hl::VertexInputInfo
						{
							.attributes =
							{
								{
									.name = "inPosition",
									.format = hl::VertexAttributeFormat::Vec2,
									.location = 0,
									.offset = offsetof(hl::VertexUi2, pos)
								},
								{
									.name = "inColor",
									.format = hl::VertexAttributeFormat::Vec4,
									.location = 1,
									.offset = offsetof(hl::VertexUi2, color)
								},
								{
									.name = "inTexCoord",
									.format = hl::VertexAttributeFormat::Vec2,
									.location = 2,
									.offset = offsetof(hl::VertexUi2, texCoord)
								},
								{
									.name = "inTexIndex",
									.format = hl::VertexAttributeFormat::Float,
									.location = 3,
									.offset = offsetof(hl::VertexUi2, texIndex)
								}
							},
							.stride = sizeof(hl::VertexUi2)
						},
						.depthState =
						{
							.testEnable = false,
							.writeEnable = false
						},
						.rasterState =
						{
							.cullMode = VK_CULL_MODE_NONE
						},
						.enableBlending = true,
					}
				}
			}
		};
	}

	void loadMenuUiSheet(
		hl::ResourceManager& resourceManager,
		hl::ResourceContext& resourceContext,
		const std::string& sheetName,
		const std::vector<hl::ResourceDefinition::Child>& textures)
	{
		resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
			hl::MaterialSystem::FallbackTextureName,
			resourceContext);

		resourceManager.LoadAs<hl::SignedDistanceFieldFontResource, hl::FontResource>(
			"roboto",
			resourceContext);

		const hl::ResourceDefinition definition
		{
			.name = sheetName,
			.type = "logical",
			.resources = textures
		};

		resourceManager.LoadLogical(definition, [&](const hl::ResourceDefinition::Child& child)
			{
				if (child.type != "texture")
				{
					return false;
				}

				resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
					child.name,
					resourceContext);

				return resourceManager.HasResource<hl::ImageSamplerResource>(child.name);
			});
	}

	hl::ui::Pointer readMenuPointer(hl::Engine& engine)
	{
		const auto window = engine.getInputManager().getWindowSize();
		const auto fb = engine.getInputManager().getFramebufferSize();
		auto mouse = engine.getInputManager().getMousePosition();
		if (window.x > 0.0f && window.y > 0.0f)
		{
			mouse.x *= fb.x / window.x;
			mouse.y *= fb.y / window.y;
		}

		return hl::ui::Pointer
		{
			.position = mouse,
			.primaryDown = engine.getInputManager().isButtonDown(GLFW_MOUSE_BUTTON_1),
			.primaryReleased = engine.getInputManager().isButtonReleased(GLFW_MOUSE_BUTTON_1)
		};
	}
}
