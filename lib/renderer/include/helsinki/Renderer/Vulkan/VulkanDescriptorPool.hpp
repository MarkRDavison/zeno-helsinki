#pragma once

#include <helsinki/Renderer/Export.hpp>

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>

namespace hl
{
	class HELSINKI_RENDERER_EXPORT VulkanDescriptorPool
	{

	public:
		VulkanDescriptorPool(VulkanDevice& device);

		void create();
		void destroy();

	public: // private: TODO: to private
		VulkanDevice& _device;

		VkDescriptorPool _descriptorPool{ VK_NULL_HANDLE };
	};
}