#pragma once

#include <vulkan/vulkan.h>
#include <helsinki/System/Resource/Resource.hpp>
#include <helsinki/Renderer/Vulkan/VulkanBuffer.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>

namespace hl
{

	class StorageBufferResource : public Resource
	{
	public:
		explicit StorageBufferResource(
			const std::string& id,
			ResourceContext& context,
			VkDeviceSize itemSize,
			uint32_t itemCount);

		bool Load() override;
		void Unload() override;

		VulkanBuffer& getBuffer();
		void writeToBuffer(void* data, size_t index = 0);

	private:
		VulkanDevice& _device;
		VulkanBuffer _buffer;
		VkDeviceSize _itemSize;
		uint32_t _itemCount;
		void* _mappedMemory{ nullptr };
	};

}