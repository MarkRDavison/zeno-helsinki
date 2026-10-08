#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSemaphore.hpp>
#include <array>
#include <cstdint>
#include <vector>

namespace hl
{
	class VulkanSynchronisationContext
	{
	public:
		VulkanSynchronisationContext(VulkanDevice& device);

		void create();
		void destroy();

		void waitFrame(uint32_t frameIndex);
		void waitLastSubmitted();
		uint64_t peekNextSignalValue() const;
		void onSubmitSucceeded(uint32_t frameIndex, uint64_t signalValue);

		VulkanSemaphore& getImageAvailableSemaphore(uint32_t frameIndex);
		VulkanSemaphore& getRenderFinishedSemaphore(uint32_t frameIndex);
		VulkanSemaphore& graphicsTimeline();

	public: // private: TODO to private
		VulkanDevice& _device;

		std::vector<VulkanSemaphore> _imageAvailableSemaphores;
		std::vector<VulkanSemaphore> _renderFinishedSemaphores;
		VulkanSemaphore _graphicsTimeline;
		uint64_t _lastSignaledValue{ 0 };
		std::array<uint64_t, MAX_FRAMES_IN_FLIGHT> _slotSignaledValue{};
	};
}
