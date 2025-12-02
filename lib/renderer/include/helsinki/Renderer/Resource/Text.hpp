#pragma once

#include <helsinki/System/glm.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Renderer/Vulkan/VulkanBuffer.hpp>
#include <vector>

namespace hl
{
	struct Text
	{
		Text(
			VulkanDevice& device
		) :
			_vertexCount(0),
			_vertexBuffer(device)
		{

		}

		uint32_t _vertexCount;
		VulkanBuffer _vertexBuffer;
		FontType _fontType{ FontType::Rasterised };
	};

}