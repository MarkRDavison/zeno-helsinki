#include <helsinki/Renderer/Vulkan/VulkanFence.hpp>
#include <stdexcept>
#include <iostream>

namespace hl
{

	VulkanFence::VulkanFence(
		VulkanDevice& device
	) : 
		_device(device)
	{

	}

	void VulkanFence::create()
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(_device._device, &fenceInfo, nullptr, &_fence) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}

	void VulkanFence::destroy()
	{
		vkDestroyFence(_device._device, _fence, nullptr);
	}

	void VulkanFence::wait()
	{
		vkWaitForFences(_device._device, 1, &_fence, VK_TRUE, UINT64_MAX);
	}

	void VulkanFence::reset()
	{
		vkResetFences(_device._device, 1, &_fence);
	}

}