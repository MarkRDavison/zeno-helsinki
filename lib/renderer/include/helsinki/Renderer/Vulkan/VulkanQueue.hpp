#pragma once

#include <helsinki/Renderer/Export.hpp>

#include <vulkan/vulkan.h>
#include <helsinki/Renderer/Vulkan/VulkanQueueFamilyIndices.hpp>

namespace hl
{
	class HELSINKI_RENDERER_EXPORT VulkanQueue
	{
	public:
		static VulkanQueueFamilyIndices findQueueFamilies(VkPhysicalDevice d, VkSurfaceKHR s);

	public: // private:
		VkQueue _queue{ VK_NULL_HANDLE };
	};
}