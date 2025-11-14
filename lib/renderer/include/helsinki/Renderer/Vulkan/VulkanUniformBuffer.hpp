#pragma once

#include <helsinki/Renderer/Vulkan/VulkanBuffer.hpp>

namespace hl
{

	class VulkanUniformBuffer
	{
	public:
		VulkanUniformBuffer(VulkanDevice& device);

		void create(VkDeviceSize size);
		void destroy();

		void writeToBuffer(void* data);

	public: // private TODO: to private
		VulkanDevice& _device;
		VulkanBuffer _buffer;
		void* _mappedMemory{ nullptr };
		size_t _size;
	};

}