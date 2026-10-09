#include <helsinki/Engine/Ui/UiBatch.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>

#include <algorithm>
#include <cmath>

constexpr auto MAX_UI_VERTEXES = 4096;

namespace hl
{
	namespace
	{
		VkRect2D boxToScissor(const hl::ui::Box& box)
		{
			const auto x0 = static_cast<int32_t>(std::floor(box.pos.x));
			const auto y0 = static_cast<int32_t>(std::floor(box.pos.y));
			const auto x1 = static_cast<int32_t>(std::ceil(box.pos.x + box.size.x));
			const auto y1 = static_cast<int32_t>(std::ceil(box.pos.y + box.size.y));
			const auto width = static_cast<uint32_t>(std::max(0, x1 - x0));
			const auto height = static_cast<uint32_t>(std::max(0, y1 - y0));
			return VkRect2D{ { x0, y0 }, { width, height } };
		}

		VkRect2D intersectScissor(VkRect2D a, VkRect2D b)
		{
			const auto left = std::max(a.offset.x, b.offset.x);
			const auto top = std::max(a.offset.y, b.offset.y);
			const auto right = std::min(
				a.offset.x + static_cast<int32_t>(a.extent.width),
				b.offset.x + static_cast<int32_t>(b.extent.width));
			const auto bottom = std::min(
				a.offset.y + static_cast<int32_t>(a.extent.height),
				b.offset.y + static_cast<int32_t>(b.extent.height));
			if (right <= left || bottom <= top)
			{
				return VkRect2D{ { left, top }, { 0, 0 } };
			}

			return VkRect2D{
				{ left, top },
				{
					static_cast<uint32_t>(right - left),
					static_cast<uint32_t>(bottom - top)
				}
			};
		}
	}

	void UiBatch::initialise(VulkanDevice& device)
	{
		for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			_mappedBuffers.emplace_back(device);
			_mappedBuffers.back().create(sizeof(hl::VertexUi2) * MAX_UI_VERTEXES, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		}
	}

	void UiBatch::setFullScissor(VkRect2D scissor)
	{
		_fullScissor = scissor;
	}

	VkRect2D UiBatch::currentScissor() const
	{
		if (_clipStack.empty())
		{
			return _fullScissor;
		}

		return _clipStack.back();
	}

	void UiBatch::startRange(VkRect2D scissor)
	{
		if (!_ranges.empty() && _ranges.back().count == 0)
		{
			_ranges.back().first = static_cast<uint32_t>(_vertices.size());
			_ranges.back().scissor = scissor;
			return;
		}

		_ranges.push_back(DrawRange{
			.first = static_cast<uint32_t>(_vertices.size()),
			.count = 0,
			.scissor = scissor });
	}

	void UiBatch::addVertexCount(uint32_t count)
	{
		if (_ranges.empty())
		{
			startRange(currentScissor());
		}

		_ranges.back().count += count;
	}

	void UiBatch::begin()
	{
		_vertices.clear();
		_ranges.clear();
		_clipStack.clear();
		_clipUsed = false;
		startRange(currentScissor());
	}

	void UiBatch::pushClip(const hl::ui::Box& worldBox)
	{
		_clipUsed = true;
		_clipStack.push_back(intersectScissor(currentScissor(), boxToScissor(worldBox)));
		startRange(currentScissor());
	}

	void UiBatch::popClip()
	{
		if (!_clipStack.empty())
		{
			_clipStack.pop_back();
		}

		startRange(currentScissor());
	}

	void UiBatch::addQuad(const hl::ui::Box& box, glm::vec4 colour, glm::vec4 texCoords, float texIndex)
	{
		_vertices.push_back(hl::VertexUi2{
			.pos = { box.pos.x, box.pos.y },
			.color = colour,
			.texCoord = glm::vec2{ texCoords.x, texCoords.y },
			.texIndex = texIndex,
			.sdf = 0.0f });
		_vertices.push_back(hl::VertexUi2{
			.pos = { box.pos.x + box.size.x, box.pos.y },
			.color = colour,
			.texCoord = glm::vec2{ texCoords.x + texCoords.z, texCoords.y },
			.texIndex = texIndex,
			.sdf = 0.0f });
		_vertices.push_back(hl::VertexUi2{
			.pos = { box.pos.x + box.size.x, box.pos.y + box.size.y },
			.color = colour,
			.texCoord = glm::vec2{ texCoords.x + texCoords.z, texCoords.y + texCoords.w },
			.texIndex = texIndex,
			.sdf = 0.0f });

		_vertices.push_back(hl::VertexUi2{
			.pos = { box.pos.x, box.pos.y },
			.color = colour,
			.texCoord = glm::vec2{ texCoords.x, texCoords.y },
			.texIndex = texIndex,
			.sdf = 0.0f });
		_vertices.push_back(hl::VertexUi2{
			.pos = { box.pos.x + box.size.x, box.pos.y + box.size.y },
			.color = colour,
			.texCoord = glm::vec2{ texCoords.x + texCoords.z, texCoords.y + texCoords.w },
			.texIndex = texIndex,
			.sdf = 0.0f });
		_vertices.push_back(hl::VertexUi2{
			.pos = { box.pos.x, box.pos.y + box.size.y },
			.color = colour,
			.texCoord = glm::vec2{ texCoords.x, texCoords.y + texCoords.w },
			.texIndex = texIndex,
			.sdf = 0.0f });
		addVertexCount(6);
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
				.texIndex = texIndex,
				.sdf = 1.0f });
		}

		addVertexCount(static_cast<uint32_t>(glyphs.size()));
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

		if (!_clipUsed)
		{
			vkCmdDraw(pdd.commandBuffer, vertexCount, 1, 0, 0);
			return;
		}

		for (const auto& range : _ranges)
		{
			if (range.count == 0)
			{
				continue;
			}

			vkCmdSetScissor(pdd.commandBuffer, 0, 1, &range.scissor);
			vkCmdDraw(pdd.commandBuffer, range.count, 1, range.first, 0);
		}
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
