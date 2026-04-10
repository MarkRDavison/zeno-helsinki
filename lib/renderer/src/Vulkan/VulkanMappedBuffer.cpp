#include <helsinki/Renderer/Vulkan/VulkanMappedBuffer.hpp>
#include <cassert>

namespace hl
{
	VulkanMappedBuffer::VulkanMappedBuffer(
		VulkanDevice& device
	) :
		_device(device)
	{

	}

	void VulkanMappedBuffer::create(VkDeviceSize size, VkBufferUsageFlags usage)
	{
		_size = size;

		VkMemoryPropertyFlags memoryPropertyFlags = 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

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
		allocInfo.memoryTypeIndex = _device.findMemoryType(memRequirements.memoryTypeBits, memoryPropertyFlags);

		CHECK_VK_RESULT(vkAllocateMemory(_device._device, &allocInfo, nullptr, &_memory));

		CHECK_VK_RESULT(vkBindBufferMemory(_device._device, _buffer, _memory, 0));

		CHECK_VK_RESULT(vkMapMemory(
			_device._device,
			_memory,
			0,
			VK_WHOLE_SIZE,
			0,
			&_mappedMemory));
	}
	
	void VulkanMappedBuffer::write(
		const void* data,
		size_t size,
		size_t offset /*= 0*/)
	{
		assert(offset + size <= _size);

		// TODO: If offset > 0, then need to make sure the data is aligned.

		memcpy(
			static_cast<char*>(_mappedMemory) + offset,
			data,
			size);
	}

	VkDeviceSize VulkanMappedBuffer::getMappedMemorySize() const
	{
		return _size;
	}

	void VulkanMappedBuffer::destroy()
	{
		if (_mappedMemory)
		{
			vkUnmapMemory(_device._device, _memory);
			_mappedMemory = nullptr;
		}

		vkDestroyBuffer(_device._device, _buffer, nullptr);
		vkFreeMemory(_device._device, _memory, nullptr);
		_size = 0;
	}
}