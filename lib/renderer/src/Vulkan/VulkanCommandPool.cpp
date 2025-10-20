#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>
#include <stdexcept>
#include <iostream>

namespace hl
{
	VulkanCommandPool::VulkanCommandPool(
		VulkanDevice & device
	) : _device(device)
	{

	}

	void VulkanCommandPool::create()
	{		
		auto queueFamilyIndices = hl::VulkanQueue::findQueueFamilies(_device._physicalDevice, _device._surface._surface);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(_device._device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics command pool!");
		}
	}

	void VulkanCommandPool::destroy()
	{
		vkDestroyCommandPool(_device._device, _commandPool, nullptr);
	}

	VulkanCommandBuffer VulkanCommandPool::createSingleTimeCommands()
	{
		VulkanCommandBuffer buffer(_device);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = _commandPool;
		allocInfo.commandBufferCount = 1;

		vkAllocateCommandBuffers(_device._device, &allocInfo, &buffer._commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(buffer._commandBuffer, &beginInfo);

		return buffer;
	}

	void VulkanCommandPool::endSingleTimeCommands(VulkanCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer._commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer._commandBuffer;

		vkQueueSubmit(_device._graphicsQueue._queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(_device._graphicsQueue._queue);

		vkFreeCommandBuffers(_device._device, _commandPool, 1, &commandBuffer._commandBuffer);
	}

}