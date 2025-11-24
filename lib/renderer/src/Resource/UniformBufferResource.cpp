#include <helsinki/Renderer/Resource/UniformBufferResource.hpp>

namespace hl
{

	UniformBufferResource::UniformBufferResource(
		const std::string& id,
		ResourceContext& context,
		VkDeviceSize size,
		uint32_t count,
		uint32_t multiple
	) : 
		Resource(id),
		_device(*context.device),
		_size(size),
		_count(count),
		_multiple(multiple)
	{

	}

	bool UniformBufferResource::Load()
	{
		_buffers.resize(_count, _device);

		for (auto& b : _buffers)
		{
			b.create(_size, _multiple);
		}

		return Resource::Load();
	}

	void UniformBufferResource::Unload()
	{
		if (IsLoaded())
		{
			for (auto& b : _buffers)
			{
				b.destroy();
			}
			_buffers.clear();
			Resource::Unload();
		}
	}

	VulkanUniformBuffer& UniformBufferResource::getUniformBuffer(uint32_t frame)
	{
		return _buffers[frame];
	}

}