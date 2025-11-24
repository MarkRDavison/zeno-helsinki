#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>

namespace hl
{

	struct ResourceContext
	{
		VulkanDevice* device{ nullptr };
		VulkanCommandPool* pool{ nullptr };
		ResourceManager* resourceManager{ nullptr };
		std::string rootPath{};
	};

}