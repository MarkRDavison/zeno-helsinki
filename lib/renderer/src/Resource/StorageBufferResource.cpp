#include <helsinki/Renderer/Resource/StorageBufferResource.hpp>

namespace hl
{

	StorageBufferResource::StorageBufferResource(
		const std::string& id,
		ResourceContext& context,
		VkDeviceSize itemSize,
		uint32_t itemCount
	) :
		Resource(id),
		_device(*context.device),
		_buffer(*context.device),
		_itemSize(itemSize),
		_itemCount(itemCount)
	{

	}

	bool StorageBufferResource::Load()
	{
		_buffer.create(
			_itemSize * _itemCount, 
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		vkMapMemory(
			_device._device,
			_buffer._memory,
			0,
			_buffer._size,
			0,
			&_mappedMemory);

		return Resource::Load();
	}

	void StorageBufferResource::Unload()
	{
		if (IsLoaded())
		{
			_buffer.destroy();
			Resource::Unload();
		}
	}

	VulkanBuffer& StorageBufferResource::getBuffer()
	{
		return _buffer;
	}

	void StorageBufferResource::writeToBuffer(void* data, size_t index /*= 0*/)
	{
		uint8_t* dst = static_cast<uint8_t*>(_mappedMemory) + index * _itemSize;
		memcpy(dst, data, _itemSize);
	}
}