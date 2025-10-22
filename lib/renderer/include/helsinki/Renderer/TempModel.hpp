#pragma once

#include <helsinki/Renderer/Vulkan/VulkanBuffer.hpp>
#include <helsinki/Renderer/Vulkan/VulkanTexture.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandBuffer.hpp>
#include <helsinki/Renderer/Vulkan/VulkanGraphicsPipeline.hpp>

namespace hl
{
	class TempModel
	{
	public:
		TempModel(VulkanDevice& device);

		void create(
			VulkanCommandPool& _commandPool,
			const std::string& modelPath, 
			const std::string& texturePath);
		void destroy();

		void draw(
			VkCommandBuffer& _buffer,
			VulkanGraphicsPipeline& _pipeline,
			VkDescriptorSet& _descriptorSet);

	public: // private: TODO: TO PRIVATE
		VulkanDevice& _device;

		uint32_t indexCount = 0;

		hl::VulkanBuffer _vertexBuffer;
		hl::VulkanBuffer _indexBuffer;
		hl::VulkanTexture _texture;
	};
}