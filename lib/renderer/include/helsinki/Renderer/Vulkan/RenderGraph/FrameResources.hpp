#pragma once

#include <unordered_map>
#include <helsinki/Renderer/RendererConfiguration.hpp>

namespace hl
{
	struct FrameResources
	{
		VkCommandBuffer primaryCmd{ VK_NULL_HANDLE };
		std::unordered_map<
			uint32_t, 
			std::vector<std::vector<VkCommandBuffer>>> secondaryCommandsByLayerAndPipelineGroup;
	};
}