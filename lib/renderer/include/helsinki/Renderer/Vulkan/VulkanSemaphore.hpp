#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>

namespace hl
{

	class VulkanSemaphore
	{
	public:
		VulkanSemaphore(VulkanDevice& device);

		void create();
		void destroy();

	public: // private: TODO to private
		VulkanDevice& _device;
		VkSemaphore _semaphore{ VK_NULL_HANDLE };
	};

}