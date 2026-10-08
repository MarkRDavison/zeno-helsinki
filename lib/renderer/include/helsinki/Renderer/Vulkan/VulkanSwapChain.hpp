#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanImage.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSurface.hpp>
#include <helsinki/Renderer/Vulkan/VulkanFramebuffer.hpp>
#include <helsinki/Renderer/Vulkan/VulkanRenderpass.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSwapChainSupportDetails.hpp>
#include <vulkan/vulkan.h>
#include <vector>

namespace hl
{
	class VulkanSwapChain
	{
	public:
		VulkanSwapChain(
			VulkanDevice& device,
			VulkanSurface& surface
		);
		void create(bool useVsync);
		void destroy();

		static VulkanSwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice p, VkSurfaceKHR s);
		static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool useVsync);
		static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow *window);

		static VkFormat findSupportedFormat(VkPhysicalDevice p, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		static VkFormat findDepthFormat(VkPhysicalDevice p);

		VkSwapchainKHR handle() const { return _swapChain; }
		VkExtent2D extent() const { return _swapChainExtent; }
		VkFormat format() const { return _swapChainImageFormat; }
		const std::vector<VkImage>& images() const { return _swapChainImages; }
		const std::vector<VkImageView>& imageViews() const { return _swapChainImageViews; }

	private:
		VulkanDevice& _device;
		VulkanSurface& _surface;

		VkSwapchainKHR _swapChain;
		std::vector<VkImage> _swapChainImages;
		VkFormat _swapChainImageFormat;
		VkExtent2D _swapChainExtent;
		std::vector<VkImageView> _swapChainImageViews;

	};
}
