#include <Views/GalaxyView.hpp>
#include <helsinki/Renderer/Resource/TextSystem.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/TextPushConstantObject.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphPipelineResources.hpp>
#include <helsinki/System/glm.hpp>

namespace slr
{

	void GalaxyView::initialise(hl::TextSystem& textSystem, int width, int height)
	{
		_labelId = textSystem.registerText("Galaxy", "roboto", 96);
		layout(textSystem, width, height);
	}

	void GalaxyView::layout(hl::TextSystem& textSystem, int width, int height)
	{
		auto desiredCenter = glm::vec2(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f);
		const auto& size = textSystem.getTextSize(_labelId);
		desiredCenter.x += size.x - size.z / 2.0f;
		desiredCenter.y += size.y - size.w / 2.0f;
		_position = glm::vec3(desiredCenter, 0.0f);
	}

	void GalaxyView::draw(hl::PipelineDrawData& pdd, hl::TextSystem& textSystem) const
	{
		const auto& t = textSystem.getText(_labelId);
		if (t._fontType != hl::FontType::SignedDistanceField)
		{
			return;
		}

		auto pc = hl::TextPushConstantObject{
			.model = glm::translate(glm::mat4(1.0f), _position),
			.colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
			.fontAtlasIndex = textSystem.getFontAtlasIndex(hl::FontType::SignedDistanceField, "roboto"),
			.cameraIndex = 0
		};

		vkCmdPushConstants(
			pdd.commandBuffer,
			pdd.pipeline->getPipelineLayout(),
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(hl::TextPushConstantObject),
			&pc);

		VkBuffer vertexBuffers[] = { t._vertexBuffer._buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(pdd.commandBuffer, 0, 1, vertexBuffers, offsets);

		auto descriptorSet = pdd.pipeline->getDescriptorSet(pdd.currentFrame);
		vkCmdBindDescriptorSets(
			pdd.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pdd.pipeline->getPipelineLayout(),
			0,
			1,
			&descriptorSet,
			0,
			nullptr);

		vkCmdDraw(pdd.commandBuffer, t._vertexCount, 1, 0, 0);
	}

}
