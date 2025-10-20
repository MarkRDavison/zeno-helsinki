#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>

namespace hl
{
	class VulkanCommandBuffer
	{
	public:
		VulkanCommandBuffer(VulkanDevice& device);

	public: // private: TODO: to private
		VulkanDevice& _device;

		VkCommandBuffer _commandBuffer{ VK_NULL_HANDLE };
	};
}