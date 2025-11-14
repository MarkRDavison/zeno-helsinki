#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandBuffer.hpp>

namespace hl
{
	class VulkanCommandPool
	{
	public:
		VulkanCommandPool(VulkanDevice& device);

		void create();
		void createTransferPool();
		void destroy();

		/*
		TODO: Separate command pools for:
			-	single command vs long loved
			-	separate queue families
			-	per thread
		*/
		VulkanCommandBuffer createSingleTimeCommands();
		void endSingleTimeCommands(VulkanCommandBuffer commandBuffer);

	public: // private: TODO: to private
		VulkanDevice& _device;

		VkCommandPool _commandPool{ VK_NULL_HANDLE };
	};
}