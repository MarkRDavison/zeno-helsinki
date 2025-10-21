#include <helsinki/Renderer/Vulkan/VulkanDescriptorPool.hpp>
#include <stdexcept>
#include <iostream>

namespace hl
{
	VulkanDescriptorPool::VulkanDescriptorPool(
		VulkanDevice& device
	) :
		_device(device)
	{

	}

	void VulkanDescriptorPool::create()
	{
		// TODO: Maybe just guesstimate these sizes, not an exact calculation
		std::vector<VkDescriptorPoolSize> poolSizes {
		   { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
		   { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20 },
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(_device._device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void VulkanDescriptorPool::destroy()
	{
		vkDestroyDescriptorPool(_device._device, _descriptorPool, nullptr);
	}
}