#include <helsinki/Renderer/Vulkan/VulkanSemaphore.hpp>
#include <stdexcept>
#include <iostream>

namespace hl
{

	VulkanSemaphore::VulkanSemaphore(
		VulkanDevice& device
	) :
		_device(device)
	{

	}

	void VulkanSemaphore::create()
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(_device._device, &semaphoreInfo, nullptr, &_semaphore) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}

	void VulkanSemaphore::destroy()
	{
		vkDestroySemaphore(_device._device, _semaphore, nullptr);
	}

}