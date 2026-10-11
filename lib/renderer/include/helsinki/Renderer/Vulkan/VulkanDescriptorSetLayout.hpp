#pragma once

#include <helsinki/Renderer/Export.hpp>

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>

namespace hl
{
	class HELSINKI_RENDERER_EXPORT VulkanDescriptorSetLayout
	{
	public:
		VulkanDescriptorSetLayout(VulkanDevice& device);

		void create(std::vector<VkDescriptorSetLayoutBinding> bindings);
		void destroy();

	public: // private: TODO: to private
		VulkanDevice& _device;

		VkDescriptorSetLayout _descriptorSetLayout{ VK_NULL_HANDLE };
	};
}