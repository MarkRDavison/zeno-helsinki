#include <helsinki/Renderer/Vulkan/VulkanSynchronisationContext.hpp>

namespace hl
{

	VulkanSynchronisationContext::VulkanSynchronisationContext(
		VulkanDevice& device
	) :
		_device(device),
		_graphicsTimeline(device)
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

			_slotSignaledValue[i] = 0;
		}

		_graphicsTimeline.createTimeline(0);
		_lastSignaledValue = 0;
	}

	void VulkanSynchronisationContext::destroy()
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			_renderFinishedSemaphores[i].destroy();
			_imageAvailableSemaphores[i].destroy();
			_slotSignaledValue[i] = 0;
		}

		_graphicsTimeline.destroy();
		_lastSignaledValue = 0;

		_renderFinishedSemaphores.clear();
		_imageAvailableSemaphores.clear();
	}

	void VulkanSynchronisationContext::waitFrame(uint32_t frameIndex)
	{
		const uint64_t value = _slotSignaledValue[frameIndex];
		if (value == 0)
		{
			return;
		}

		_graphicsTimeline.wait(value);
	}

	void VulkanSynchronisationContext::waitLastSubmitted()
	{
		if (_lastSignaledValue == 0)
		{
			return;
		}

		_graphicsTimeline.wait(_lastSignaledValue);
	}

	uint64_t VulkanSynchronisationContext::peekNextSignalValue() const
	{
		return _lastSignaledValue + 1;
	}

	void VulkanSynchronisationContext::onSubmitSucceeded(uint32_t frameIndex, uint64_t signalValue)
	{
		_slotSignaledValue[frameIndex] = signalValue;
		_lastSignaledValue = signalValue;
	}

	VulkanSemaphore& VulkanSynchronisationContext::getImageAvailableSemaphore(uint32_t frameIndex)
	{
		return _imageAvailableSemaphores[frameIndex];
	}
	VulkanSemaphore& VulkanSynchronisationContext::getRenderFinishedSemaphore(uint32_t frameIndex)
	{
		return _renderFinishedSemaphores[frameIndex];
	}
	VulkanSemaphore& VulkanSynchronisationContext::graphicsTimeline()
	{
		return _graphicsTimeline;
	}
}
