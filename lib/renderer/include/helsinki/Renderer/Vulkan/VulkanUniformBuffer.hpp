#pragma once

#include <helsinki/Renderer/Vulkan/VulkanBuffer.hpp>

namespace hl
{

	class VulkanUniformBuffer
	{
	public:
		VulkanUniformBuffer(VulkanDevice& device);

		void create(VkDeviceSize size, uint32_t multiple = 1);
		void destroy();

		void writeToBuffer(void* data, size_t index = 0);

	public: // private TODO: to private
		VulkanDevice& _device;
		VulkanBuffer _buffer;
		void* _mappedMemory{ nullptr };
		size_t _size;
		uint32_t _multiple;
	};

}