#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>

namespace hl
{

	class VulkanMappedBuffer
	{
	public:
		VulkanMappedBuffer(VulkanDevice& device); 
		
		VulkanMappedBuffer(const VulkanMappedBuffer&) = delete;
		VulkanMappedBuffer& operator=(const VulkanMappedBuffer&) = delete;
		VulkanMappedBuffer(VulkanMappedBuffer&&) noexcept = default;
		VulkanMappedBuffer& operator=(VulkanMappedBuffer&&) noexcept = default;

		void create(VkDeviceSize size, VkBufferUsageFlags usage);

		void write(
			const void* data,
			size_t size,
			size_t offset = 0);

		VkDeviceSize getMappedMemorySize() const;
		VkBuffer getBuffer() const { return _buffer; }

		void destroy();
	private:
		VulkanDevice& _device;
		VkDeviceSize _size{ 0 };

		VkBuffer _buffer{ VK_NULL_HANDLE };
		VkDeviceMemory _memory{ VK_NULL_HANDLE };
	public:
		void * _mappedMemory{ VK_NULL_HANDLE };
	};

}