#pragma once

#include <helsinki/Renderer/Export.hpp>

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <cstdint>

namespace hl
{

	class HELSINKI_RENDERER_EXPORT VulkanSemaphore
	{
	public:
		VulkanSemaphore(VulkanDevice& device);

		void create();
		void createTimeline(uint64_t initialValue = 0);
		void destroy();
		void wait(uint64_t value);

	public: // private: TODO to private
		VulkanDevice& _device;
		VkSemaphore _semaphore{ VK_NULL_HANDLE };
	};

}
