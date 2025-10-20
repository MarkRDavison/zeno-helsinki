#include <helsinki/Renderer/Vulkan/VulkanSwapChain.hpp>

namespace hl
{
	VulkanSwapChainSupportDetails VulkanSwapChain::querySwapChainSupport(VkPhysicalDevice p, VkSurfaceKHR s)
	{
        VulkanSwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p, s, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(p, s, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(p, s, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(p, s, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(p, s, &presentModeCount, details.presentModes.data());
        }

        return details;
	}
}