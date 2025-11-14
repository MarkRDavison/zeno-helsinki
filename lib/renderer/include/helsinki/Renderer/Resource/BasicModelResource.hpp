#pragma once

#include <vulkan/vulkan.h>
#include <helsinki/System/Resource/Resource.hpp>
#include <helsinki/Renderer/Vulkan/VulkanBuffer.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>

namespace hl
{

	class BasicModelResource : public Resource
	{
	public:
		explicit BasicModelResource(
			const std::string& id,
			ResourceContext& context);

		bool Load() override;
		void Unload() override;

		VkBuffer getVertexBuffer() const;
		VkBuffer getIndexBuffer() const;
		uint32_t getIndexCount() const;

	private:
		VulkanDevice& _device;
		VulkanCommandPool& _commandPool;
		const std::string _rootPath;
		hl::VulkanBuffer _vertexBuffer;
		hl::VulkanBuffer _indexBuffer;
		uint32_t _indexCount = 0;
	};

}