#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanRenderpass.hpp>
#include <helsinki/Renderer/Vulkan/VulkanPipelineLayout.hpp>

namespace hl
{

	class VulkanGraphicsPipeline
	{
	public:
		VulkanGraphicsPipeline(VulkanDevice& device);

		void create(
			const std::string& vertexPath, 
			const std::string& fragmentPath, 
			VulkanRenderpass& renderpass,
			VulkanDescriptorSetLayout& descriptorSetLayout);
		void destroy();

	public: // private: TODO: to private
		VulkanDevice& _device;
		VulkanPipelineLayout _pipelineLayout;

		VkPipeline _graphicsPipeline{VK_NULL_HANDLE};
	};

}