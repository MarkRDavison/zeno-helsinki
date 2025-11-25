#pragma once


#include <helsinki/System/glm.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Renderer/Vulkan/VulkanBuffer.hpp>
#include <string>
#include <vector>

namespace hl
{
	struct Mesh
	{
		Mesh(
			VulkanDevice& device
		) :
			_indexCount(0),
			_vertexBuffer(device),
			_indexBuffer(device)
		{

		}

		std::string materialName;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		uint32_t _indexCount;
		VulkanBuffer _vertexBuffer;
		VulkanBuffer _indexBuffer;
	};

}