#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>


namespace hl
{
	class ResourceManager;
	class MaterialSystem;
	struct ResourceContext
	{
		VulkanDevice* device{ nullptr };
		VulkanCommandPool* pool{ nullptr };
		ResourceManager* resourceManager{ nullptr };
		MaterialSystem* materialSystem{ nullptr };
		std::string rootPath{};
	};

}