#include <helsinki/Renderer/Vulkan/VulkanUniformBuffer.hpp>

namespace hl
{
	VulkanUniformBuffer::VulkanUniformBuffer(
		VulkanDevice& device
	) : 
		_device(device),
		_buffer(device),
		_size(0)
	{

	}

	void VulkanUniformBuffer::create(VkDeviceSize size)
	{
		_size = size;
		_buffer.create(
			size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		vkMapMemory(
			_device._device, 
			_buffer._memory, 
			0, 
			_buffer._size, 
			0, 
			&_mappedMemory);
	}

	void VulkanUniformBuffer::destroy()
	{
		_buffer.destroy();
	}

	void VulkanUniformBuffer::writeToBuffer(void* data)
	{
		memcpy(_mappedMemory, data, _size);
	}
}