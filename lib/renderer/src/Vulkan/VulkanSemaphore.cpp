#include <helsinki/Renderer/Vulkan/VulkanSemaphore.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>

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

		CHECK_VK_RESULT(vkCreateSemaphore(_device.handle(), &semaphoreInfo, nullptr, &_semaphore));
	}

	void VulkanSemaphore::createTimeline(uint64_t initialValue)
	{
		VkSemaphoreTypeCreateInfo typeInfo{};
		typeInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		typeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		typeInfo.initialValue = initialValue;

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = &typeInfo;

		CHECK_VK_RESULT(vkCreateSemaphore(_device.handle(), &semaphoreInfo, nullptr, &_semaphore));
	}

	void VulkanSemaphore::destroy()
	{
		vkDestroySemaphore(_device.handle(), _semaphore, nullptr);
		_semaphore = VK_NULL_HANDLE;
	}

	void VulkanSemaphore::wait(uint64_t value)
	{
		VkSemaphoreWaitInfo waitInfo{};
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		waitInfo.semaphoreCount = 1;
		waitInfo.pSemaphores = &_semaphore;
		waitInfo.pValues = &value;

		CHECK_VK_RESULT(vkWaitSemaphores(_device.handle(), &waitInfo, UINT64_MAX));
	}

}
