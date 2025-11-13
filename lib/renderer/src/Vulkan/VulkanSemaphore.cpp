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

		CHECK_VK_RESULT(vkCreateSemaphore(_device._device, &semaphoreInfo, nullptr, &_semaphore));
	}

	void VulkanSemaphore::destroy()
	{
		vkDestroySemaphore(_device._device, _semaphore, nullptr);
	}

}