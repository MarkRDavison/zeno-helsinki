#include <helsinki/Renderer/Vulkan/VulkanPipelineLayout.hpp>
#include <stdexcept>
#include <iostream>
#include <glm/glm.hpp>

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
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(glm::mat4);
		// todo: compare this size to physicalDeviceProperties.limits.maxPushConstantsSize

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		if (descriptorSetLayout._descriptorSetLayout == VK_NULL_HANDLE)
		{
			pipelineLayoutInfo.setLayoutCount = 0;
			pipelineLayoutInfo.pSetLayouts = nullptr;
		}
		else
		{
			pipelineLayoutInfo.setLayoutCount = 1;
			pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout._descriptorSetLayout;
		}
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

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