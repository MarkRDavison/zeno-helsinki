#include <helsinki/Renderer/Vulkan/VulkanGraphicsPipelineResources.hpp>

namespace hl
{

	VulkanGraphicsPipelineResources::VulkanGraphicsPipelineResources(
		VulkanDevice& device
	) :
		_device(device),
		_descriptorSetLayout(device),
		_descriptorSet(device),
		_graphicsPipeline(device)
	{
	}

	void VulkanGraphicsPipelineResources::destroy()
	{
		_graphicsPipeline.destroy();

		_descriptorSetLayout.destroy();
	}
}