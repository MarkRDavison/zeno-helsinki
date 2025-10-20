#pragma once

#include <helsinki/Renderer/Vulkan/VulkanSwapChainSupportDetails.hpp>
#include <vulkan/vulkan.h>

namespace hl
{
	class VulkanSwapChain
	{
	public:
		static VulkanSwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice p, VkSurfaceKHR s);

	private:

	};
}