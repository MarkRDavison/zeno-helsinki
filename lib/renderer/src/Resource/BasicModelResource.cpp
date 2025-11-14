#include <helsinki/Renderer/Resource/BasicModelResource.hpp>
#include <helsinki/Renderer/ModelLoader.hpp>
#include <format>

namespace hl
{

	BasicModelResource::BasicModelResource(
		const std::string& id,
		ResourceContext& context
	) : 
		Resource(id),
        _device(*context.device),
        _commandPool(*context.pool),
        _rootPath(context.rootPath),
		_vertexBuffer(*context.device),
		_indexBuffer(*context.device)
	{

	}

	bool BasicModelResource::Load()
	{
        std::string path = std::format("{}/data/models/{}.obj", _rootPath, GetId());
        auto [vertices, indices] = hl::ModelLoader::loadModel(path);

        _indexCount = (uint32_t)indices.size();

        {
            VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

            hl::VulkanBuffer stagingBuffer(_device);
            stagingBuffer.create(
                bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            stagingBuffer.mapMemory(
                vertices.data());

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
            VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

            hl::VulkanBuffer stagingBuffer(_device);

            stagingBuffer.create(
                bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            stagingBuffer.mapMemory(
                indices.data());

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

	void BasicModelResource::Unload()
	{
		if (IsLoaded())
		{
			_vertexBuffer.destroy();
			_indexBuffer.destroy();
            _indexCount = 0;
			Resource::Unload();
		}
	}

    VkBuffer BasicModelResource::getVertexBuffer() const { return _vertexBuffer._buffer; }
    VkBuffer BasicModelResource::getIndexBuffer() const { return _indexBuffer._buffer; }
    uint32_t BasicModelResource::getIndexCount() const { return _indexCount; }

}