#pragma once

#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <string>

namespace hl
{
	class ModelLoader
	{
	public:
		static std::pair<std::vector<hl::Vertex>, std::vector<uint32_t>> loadModel(const std::string& modelPath);
	};
}