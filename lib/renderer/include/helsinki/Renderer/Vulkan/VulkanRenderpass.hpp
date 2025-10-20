#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSwapChain.hpp>

namespace hl
{
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