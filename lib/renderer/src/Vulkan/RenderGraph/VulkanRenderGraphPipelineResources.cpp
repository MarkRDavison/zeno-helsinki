#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphPipelineResources.hpp>

namespace hl
{

	VulkanRenderGraphPipelineResources::VulkanRenderGraphPipelineResources(
		const std::string& name,
		VulkanDevice& device
	) :
		Name(name),
		_device(device)
	{

	}

	void VulkanRenderGraphPipelineResources::addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
	{
		_descriptorSetLayout = descriptorSetLayout;
	}
	void VulkanRenderGraphPipelineResources::addPipelineLayout(VkPipelineLayout pipelineLayout)
	{
		_pipelineLayout = pipelineLayout;
	}
	void VulkanRenderGraphPipelineResources::addPipeline(VkPipeline pipeline)
	{
		_pipeline = pipeline;
	}

	void VulkanRenderGraphPipelineResources::destroy()
	{
		vkDestroyPipeline(_device._device, _pipeline, nullptr);
		vkDestroyPipelineLayout(_device._device, _pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(_device._device, _descriptorSetLayout, nullptr);
	}

}