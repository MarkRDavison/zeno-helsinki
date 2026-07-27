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
	void VulkanRenderGraphPipelineResources::addDescriptorSets(std::vector<VkDescriptorSet> descriptorSets)
	{
		_descriptorSets = descriptorSets;
	}
	void VulkanRenderGraphPipelineResources::addViewportInfo(ViewportMode mode, uint32_t width, uint32_t height)
	{
		_mode = mode;
		_width = width;
		_height = height;
	}

	VkDescriptorSet VulkanRenderGraphPipelineResources::getDescriptorSet(uint32_t index)
	{
		return _descriptorSets[index];
	}
	VkPipeline VulkanRenderGraphPipelineResources::getPipeline() const
	{
		return _pipeline;
	}
	VkPipelineLayout VulkanRenderGraphPipelineResources::getPipelineLayout() const
	{
		return _pipelineLayout;
	}

	void VulkanRenderGraphPipelineResources::destroy()
	{
		_descriptorSets.clear();
		vkDestroyPipeline(_device._device, _pipeline, nullptr);
		vkDestroyPipelineLayout(_device._device, _pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(_device._device, _descriptorSetLayout, nullptr);
	}

}