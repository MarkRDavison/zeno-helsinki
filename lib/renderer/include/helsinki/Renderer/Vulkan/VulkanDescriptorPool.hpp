#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>

namespace hl
{
	class VulkanDescriptorPool
	{

	public:
		VulkanDescriptorPool(VulkanDevice& device);

		void create();
		void destroy();

	public: // private: TODO: to private
		VulkanDevice& _device;

		VkDescriptorPool _descriptorPool{ VK_NULL_HANDLE };
	};
}