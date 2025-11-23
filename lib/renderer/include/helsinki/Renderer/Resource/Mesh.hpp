#pragma once


#include <helsinki/System/glm.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <string>
#include <vector>

namespace hl
{

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::string materialName;
	};

}