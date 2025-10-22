#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>

namespace hl
{

	class VulkanFence
	{
	public:
		VulkanFence(VulkanDevice& device);

		void create();
		void destroy();

		void wait();
		void reset();

	public: // private: TODO to private
		VulkanDevice& _device;
		VkFence _fence{ VK_NULL_HANDLE };
	};

}