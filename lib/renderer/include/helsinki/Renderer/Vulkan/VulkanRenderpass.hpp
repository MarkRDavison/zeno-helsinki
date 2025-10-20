#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>

namespace hl
{
	class VulkanSwapChain;
	class VulkanRenderpass
	{
	public:
		VulkanRenderpass(
			VulkanDevice& device,
			VulkanSwapChain& swapChain);

		void create();
		void destroy();

	public: // private: TODO: PRIVATE
		VulkanDevice& _device;
		VulkanSwapChain& _swapChain;
		VkRenderPass _renderPass;
	};
}