#pragma once

#include <helsinki/Renderer/Export.hpp>

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDescriptorSetLayout.hpp>
#include <string>

namespace hl
{

	class HELSINKI_RENDERER_EXPORT VulkanPipelineLayout
	{
	public:
		VulkanPipelineLayout(VulkanDevice& device);

		void create(VulkanDescriptorSetLayout& descriptorSetLayout);
		void destroy();

	public: // private: TODO: to private
		VulkanDevice& _device;

		VkPipelineLayout _pipelineLayout{ VK_NULL_HANDLE};
	};

}