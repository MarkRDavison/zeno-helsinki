#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>

namespace hl
{

	struct ResourceContext
	{
		VulkanDevice* device{ nullptr };
		VulkanCommandPool* pool{ nullptr };
		std::string rootPath{};
	};

}