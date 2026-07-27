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
		void addViewportInfo(ViewportMode mode, uint32_t width, uint32_t height);

		VkDescriptorSet getDescriptorSet(uint32_t index);
		VkPipeline getPipeline() const;
		VkPipelineLayout getPipelineLayout() const;

		ViewportMode getViewportMode() const { return _mode; }
		uint32_t getViewportWidth() const { return _width; }
		uint32_t getViewportHeight() const { return _height; }

		void destroy();

	private:
		VulkanDevice& _device;
		VkDescriptorSetLayout _descriptorSetLayout{ VK_NULL_HANDLE };
		VkPipelineLayout _pipelineLayout{ VK_NULL_HANDLE };
		VkPipeline _pipeline{ VK_NULL_HANDLE };
		std::vector<VkDescriptorSet> _descriptorSets;
		ViewportMode _mode{}; 
		uint32_t _width;
		uint32_t _height;
	};

}