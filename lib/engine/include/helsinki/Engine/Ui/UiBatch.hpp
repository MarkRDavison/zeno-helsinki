#pragma once

#include <helsinki/System/Utils/NonCopyable.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Renderer/Vulkan/VulkanMappedBuffer.hpp>
#include <helsinki/Ui/Layout/Box.hpp>
#include <vector>

namespace hl
{
	struct PipelineDrawData;
	class VulkanDevice;

	class UiBatch : public NonCopyable
	{
	public:
		UiBatch() = default;

		void initialise(VulkanDevice& device);
		void begin();
		void addQuad(const hl::ui::Box& box, glm::vec4 colour, glm::vec4 texCoords, float texIndex);
		void addGlyphs(
			const std::vector<hl::Vertex22D>& glyphs,
			glm::vec2 offset,
			glm::vec3 colour,
			float texIndex);
		void updateGpuResources(uint32_t currentFrame);
		void draw(PipelineDrawData& pdd) const;
		void destroy();

	private:
		std::vector<hl::VertexUi2> _vertices;
		std::vector<VulkanMappedBuffer> _mappedBuffers;
	};
}
