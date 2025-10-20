#pragma once

#include <vulkan/vulkan.h>
#include <helsinki/Renderer/Vulkan/VulkanQueueFamilyIndices.hpp>

namespace hl
{
	class VulkanQueue
	{
	public:
		static VulkanQueueFamilyIndices findQueueFamilies(VkPhysicalDevice d, VkSurfaceKHR s);

	public: // private:
		VkQueue _queue{ VK_NULL_HANDLE };
	};
}