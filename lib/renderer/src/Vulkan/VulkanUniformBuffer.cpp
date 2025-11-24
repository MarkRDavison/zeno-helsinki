#include <helsinki/Renderer/Vulkan/VulkanUniformBuffer.hpp>

namespace hl
{
	VulkanUniformBuffer::VulkanUniformBuffer(
		VulkanDevice& device
	) : 
		_device(device),
		_buffer(device),
		_size(0),
		_multiple(0)
	{

	}

	void VulkanUniformBuffer::create(VkDeviceSize size, uint32_t multiple /*= 1*/)
	{
		_size = size;
		_multiple = multiple;
		_buffer.create(
			size * multiple,
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

	void VulkanUniformBuffer::writeToBuffer(void* data, size_t index /*= 0*/)
	{
		uint8_t* dst = static_cast<uint8_t*>(_mappedMemory) + index * _size;
		memcpy(dst, data, _size);
	}
}