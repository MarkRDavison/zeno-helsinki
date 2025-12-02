#include <helsinki/Renderer/Vulkan/VulkanBuffer.hpp>
#include <stdexcept>
#include <iostream>

namespace hl
{
	VulkanBuffer::VulkanBuffer(
		VulkanDevice& device
	) :
		_device(device)
	{

	}

	void VulkanBuffer::create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		_size = size;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		CHECK_VK_RESULT(vkCreateBuffer(_device._device, &bufferInfo, nullptr, &_buffer));

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(_device._device, _buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = _device.findMemoryType(memRequirements.memoryTypeBits, properties);

		CHECK_VK_RESULT(vkAllocateMemory(_device._device, &allocInfo, nullptr, &_memory));

		CHECK_VK_RESULT(vkBindBufferMemory(_device._device, _buffer, _memory, 0));
	}

	void VulkanBuffer::mapMemory(const void* data)
	{
		void* mappedMemory;
		CHECK_VK_RESULT(vkMapMemory(_device._device, _memory, 0, _size, 0, &mappedMemory));
		memcpy(mappedMemory, data, static_cast<size_t>(_size));
		vkUnmapMemory(_device._device, _memory);
	}

	void VulkanBuffer::copyToBuffer(VulkanCommandPool& commandPool, VkDeviceSize size, VulkanBuffer& target)
	{
		auto commandBuffer = commandPool.createSingleTimeCommands();

		// TODO: Validate buffer size
		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer._commandBuffer, _buffer, target._buffer, 1, &copyRegion);

		commandPool.endSingleTimeCommands(commandBuffer);
	}

	void VulkanBuffer::destroy()
	{
		vkDestroyBuffer(_device._device, _buffer, nullptr);
		vkFreeMemory(_device._device, _memory, nullptr);
		_size = 0;
	}
}