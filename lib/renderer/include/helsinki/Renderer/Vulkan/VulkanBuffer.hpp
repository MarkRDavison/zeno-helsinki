#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>

namespace hl
{
	class VulkanBuffer
	{
	public:
		VulkanBuffer(VulkanDevice& device);

		void create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

		void mapMemory(void* data);

		void destroy();

	public: //private: TODO: TO PRIVATE
		VulkanDevice& _device;
		VkDeviceSize _size{ 0 };

		VkBuffer _buffer{ VK_NULL_HANDLE };
		VkDeviceMemory _memory{ VK_NULL_HANDLE };
	};
}