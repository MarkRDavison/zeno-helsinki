#pragma once

#include <helsinki/Renderer/Export.hpp>

#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <string>

namespace hl
{
	class HELSINKI_RENDERER_EXPORT ModelLoader
	{
	public:
		static std::pair<std::vector<hl::Vertex>, std::vector<uint32_t>> loadModel(const std::string& modelPath);
	};
}