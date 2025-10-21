#include <helsinki/Renderer/Vulkan/VulkanPipelineLayout.hpp>
#include <stdexcept>
#include <iostream>

namespace hl
{
	VulkanPipelineLayout::VulkanPipelineLayout(
		VulkanDevice& device
	) :
		_device(device)
	{

	}

	void VulkanPipelineLayout::create(VulkanDescriptorSetLayout& descriptorSetLayout)
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout._descriptorSetLayout;

		if (vkCreatePipelineLayout(_device._device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void VulkanPipelineLayout::destroy()
	{
		vkDestroyPipelineLayout(_device._device, _pipelineLayout, nullptr);
	}	
}