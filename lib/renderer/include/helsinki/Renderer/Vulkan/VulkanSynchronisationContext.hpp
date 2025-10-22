#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSemaphore.hpp>
#include <helsinki/Renderer/Vulkan/VulkanFence.hpp>

namespace hl
{
	class VulkanSynchronisationContext
	{
	public:
		VulkanSynchronisationContext(VulkanDevice& device);

		void create();
		void destroy();

		VulkanFence& getFence(uint32_t frameIndex);
		VulkanSemaphore& getImageAvailableSemaphore(uint32_t frameIndex);
		VulkanSemaphore& getRenderFinishedSemaphore(uint32_t frameIndex);

	public: // private: TODO to private
		VulkanDevice& _device;

		std::vector<VulkanFence> _fences;
		std::vector<VulkanSemaphore> _imageAvailableSemaphores;
		std::vector<VulkanSemaphore> _renderFinishedSemaphores;
	};
}