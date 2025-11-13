#include <helsinki/Renderer/Vulkan/VulkanDescriptorSetLayout.hpp>
#include <stdexcept>
#include <iostream>

namespace hl
{

	VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(
		VulkanDevice& device
	) :
		_device(device)
	{

	}

	void VulkanDescriptorSetLayout::create(std::vector<VkDescriptorSetLayoutBinding> bindings)
	{
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

		CHECK_VK_RESULT(vkCreateDescriptorSetLayout(_device._device, &layoutInfo, nullptr, &_descriptorSetLayout));
	}

	void VulkanDescriptorSetLayout::destroy()
	{
		vkDestroyDescriptorSetLayout(_device._device, _descriptorSetLayout, nullptr);
	}

}