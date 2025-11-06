#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>

namespace hl
{
	class VulkanSwapChain;
	class VulkanRenderpass
	{
	public:
		VulkanRenderpass(
			VulkanDevice& device);

		void createBasicRenderpass(bool multiSample, VkFormat imageFormat);
		void createBasicRenderpassWithFollowingRenderpass(bool multiSample, VkFormat imageFormat);
		void destroy();

	private:
		void createBasicRenderpassInternal(bool multiSample, bool writeToSwapchain, VkFormat imageFormat);

	public: // private: TODO: PRIVATE
		VulkanDevice& _device;
		VkRenderPass _renderPass{ VK_NULL_HANDLE };
	};
}