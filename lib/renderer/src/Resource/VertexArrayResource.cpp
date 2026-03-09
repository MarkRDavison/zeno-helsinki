#include <helsinki/Renderer/Resource/VertexArrayResource.hpp>

namespace hl
{

	VertexArrayResource::VertexArrayResource(
		const std::string& id,
		ResourceContext& context,
		const std::vector<hl::Vertex2>& verticies
	) :
		Resource(id),
		_device(*context.device),
		_commandPool(*context.pool),
		_rootPath(context.rootPath),
		_verticies(verticies),
		_vertexBuffer(*context.device),
		_indexBuffer(*context.device)
	{

	}

	bool VertexArrayResource::Load()
	{
		_indices.clear();

		{
			VkDeviceSize bufferSize = sizeof(_verticies[0]) * _verticies.size();

			hl::VulkanBuffer stagingBuffer(_device);

			stagingBuffer.create(
				bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			stagingBuffer.mapMemory(
				_verticies.data());

			_vertexBuffer.create(
				bufferSize,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			stagingBuffer.copyToBuffer(
				_commandPool,
				bufferSize,
				_vertexBuffer);

			stagingBuffer.destroy();
		}
		{
			for (uint32_t i = 0; i < _verticies.size(); ++i)
			{
				// TODO: algorithm thats better
				_indices.push_back(i);
			}

			VkDeviceSize bufferSize = sizeof(_indices[0]) * _indices.size();

			hl::VulkanBuffer stagingBuffer(_device);

			stagingBuffer.create(
				bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			stagingBuffer.mapMemory(
				_indices.data());

			_indexBuffer.create(
				bufferSize,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			stagingBuffer.copyToBuffer(
				_commandPool,
				bufferSize,
				_indexBuffer);

			stagingBuffer.destroy();
		}

		return Resource::Load();
	}

	void VertexArrayResource::Unload()
	{
		if (IsLoaded())
		{
			_vertexBuffer.destroy();
			_indexBuffer.destroy();
			Resource::Unload();
		}
	}
}