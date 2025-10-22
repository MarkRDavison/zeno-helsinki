#include <helsinki/Renderer/TempModel.hpp>
#include <helsinki/Renderer/ModelLoader.hpp>

namespace hl
{
	TempModel::TempModel(
		VulkanDevice& device
	) :
		_device(device),
		_vertexBuffer(device),
		_indexBuffer(device),
        _texture(device)
	{

	}

	void TempModel::create(
        VulkanCommandPool& _commandPool,
        const std::string& modelPath,
        const std::string& texturePath)
	{
        _texture.create(_commandPool, texturePath);

		auto [vertices, indices] = hl::ModelLoader::loadModel(modelPath);

		indexCount = (uint32_t)indices.size();

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
	}

	void TempModel::destroy()
	{
        _texture.destroy();
        _vertexBuffer.destroy();
        _indexBuffer.destroy();
	}

    void TempModel::draw(
        VkCommandBuffer& _buffer,
        VulkanGraphicsPipeline& _pipeline,
        VkDescriptorSet& _descriptorSet)
    {
        VkBuffer vertexBuffers[] = { _vertexBuffer._buffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(
            _buffer, 
            0, 
            1, 
            vertexBuffers, 
            offsets);

        vkCmdBindIndexBuffer(
            _buffer, 
            _indexBuffer._buffer, 
            0, 
            VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(
            _buffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS, 
            _pipeline._pipelineLayout._pipelineLayout,
            0, 
            1, 
            &_descriptorSet, 
            0, 
            nullptr);

        vkCmdDrawIndexed(_buffer, indexCount, 1, 0, 0, 0);
    }
}