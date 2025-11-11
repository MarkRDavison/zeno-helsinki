#pragma once

#include <string>
#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanGraphicsPipeline.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>

namespace hl
{

	class VulkanRenderGraphPipelineResources
	{
	public:
		VulkanRenderGraphPipelineResources(
			const std::string& name, 
			VulkanDevice& device);

		const std::string Name;

		void addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
		void addPipelineLayout(VkPipelineLayout pipelineLayout);
		void addPipeline(VkPipeline pipeline);
		void addDescriptorSets(std::vector<VkDescriptorSet> descriptorSets);

		VkDescriptorSet getDescriptorSet(uint32_t index);
		VkPipeline getPipeline() const;
		VkPipelineLayout getPipelineLayout() const;

		void destroy();

	private:
		VulkanDevice& _device;
		VkDescriptorSetLayout _descriptorSetLayout{ VK_NULL_HANDLE };
		VkPipelineLayout _pipelineLayout{ VK_NULL_HANDLE };
		VkPipeline _pipeline{ VK_NULL_HANDLE };
		std::vector<VkDescriptorSet> _descriptorSets;
	};

}