#include <helsinki/Renderer/Vulkan/VulkanSynchronisationContext.hpp>

namespace hl
{

	VulkanSynchronisationContext::VulkanSynchronisationContext(
		VulkanDevice& device
	) :
		_device(device)
	{

	}

	void VulkanSynchronisationContext::create()
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			_imageAvailableSemaphores.emplace_back(_device);
			_imageAvailableSemaphores.back().create();

			_renderFinishedSemaphores.emplace_back(_device);
			_renderFinishedSemaphores.back().create();

			_fences.emplace_back(_device);
			_fences.back().create();
		}
	}

	void VulkanSynchronisationContext::destroy()
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			_renderFinishedSemaphores[i].destroy();
			_imageAvailableSemaphores[i].destroy();
			_fences[i].destroy();
		}

		_renderFinishedSemaphores.clear();
		_imageAvailableSemaphores.clear();
		_fences.clear();
	}

	VulkanFence& VulkanSynchronisationContext::getFence(uint32_t frameIndex)
	{
		return _fences[frameIndex];
	}
	VulkanSemaphore& VulkanSynchronisationContext::getImageAvailableSemaphore(uint32_t frameIndex)
	{
		return _imageAvailableSemaphores[frameIndex];
	}
	VulkanSemaphore& VulkanSynchronisationContext::getRenderFinishedSemaphore(uint32_t frameIndex)
	{
		return _renderFinishedSemaphores[frameIndex];
	}
}