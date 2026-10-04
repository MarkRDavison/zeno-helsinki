#include <helsinki/Engine/Ui/UiBatch.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>

constexpr auto MAX_UI_VERTEXES = 4096;

namespace hl
{
	void UiBatch::initialise(VulkanDevice& device)
	{
		for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			_mappedBuffers.emplace_back(device);
			_mappedBuffers.back().create(sizeof(hl::VertexUi2) * MAX_UI_VERTEXES, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		}
	}

	void UiBatch::begin()
	{
		_vertices.clear();
	}

	void UiBatch::addQuad(const hl::ui::Box& box, glm::vec4 colour, glm::vec4 texCoords, float texIndex)
	{
		_vertices.push_back(hl::VertexUi2{
			.pos = { box.pos.x, box.pos.y },
			.color = colour,
			.texCoord = glm::vec2{ texCoords.x, texCoords.y },
			.texIndex = texIndex });
		_vertices.push_back(hl::VertexUi2{
			.pos = { box.pos.x + box.size.x, box.pos.y },
			.color = colour,
			.texCoord = glm::vec2{ texCoords.x + texCoords.z, texCoords.y },
			.texIndex = texIndex });
		_vertices.push_back(hl::VertexUi2{
			.pos = { box.pos.x + box.size.x, box.pos.y + box.size.y },
			.color = colour,
			.texCoord = glm::vec2{ texCoords.x + texCoords.z, texCoords.y + texCoords.w },
			.texIndex = texIndex });

		_vertices.push_back(hl::VertexUi2{
			.pos = { box.pos.x, box.pos.y },
			.color = colour,
			.texCoord = glm::vec2{ texCoords.x, texCoords.y },
			.texIndex = texIndex });
		_vertices.push_back(hl::VertexUi2{
			.pos = { box.pos.x + box.size.x, box.pos.y + box.size.y },
			.color = colour,
			.texCoord = glm::vec2{ texCoords.x + texCoords.z, texCoords.y + texCoords.w },
			.texIndex = texIndex });
		_vertices.push_back(hl::VertexUi2{
			.pos = { box.pos.x, box.pos.y + box.size.y },
			.color = colour,
			.texCoord = glm::vec2{ texCoords.x, texCoords.y + texCoords.w },
			.texIndex = texIndex });
	}

	void UiBatch::addGlyphs(
		const std::vector<hl::Vertex22D>& glyphs,
		glm::vec2 offset,
		glm::vec3 colour,
		float texIndex)
	{
		for (const auto& glyph : glyphs)
		{
			_vertices.push_back(hl::VertexUi2{
				.pos = glyph.pos + offset,
				.color = glm::vec4{ colour, 1.0f },
				.texCoord = glyph.texCoord,
				.texIndex = texIndex });
		}
	}

	void UiBatch::updateGpuResources(uint32_t currentFrame)
	{
		const auto size = _vertices.size() * sizeof(hl::VertexUi2);
		if (size > 0)
		{
			_mappedBuffers[currentFrame].write(_vertices.data(), size);
		}
	}

	void UiBatch::draw(PipelineDrawData& pdd) const
	{
		const auto vertexCount = static_cast<uint32_t>(_vertices.size());
		if (vertexCount == 0)
		{
			return;
		}

		VkBuffer vertexBuffers[] = { _mappedBuffers[pdd.currentFrame].getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(
			pdd.commandBuffer,
			0,
			1,
			vertexBuffers,
			offsets);

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

		vkCmdDraw(pdd.commandBuffer, vertexCount, 1, 0, 0);
	}

	void UiBatch::destroy()
	{
		for (auto& mb : _mappedBuffers)
		{
			mb.destroy();
		}

		_mappedBuffers.clear();
	}
}
