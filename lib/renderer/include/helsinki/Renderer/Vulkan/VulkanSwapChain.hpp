#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSurface.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSwapChainSupportDetails.hpp>
#include <vulkan/vulkan.h>

namespace hl
{
	class VulkanSwapChain
	{
	public:
		VulkanSwapChain(
			VulkanDevice& device,
			VulkanSurface& surface
		);
		void create();
		void destroy();

		static VulkanSwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice p, VkSurfaceKHR s);
		static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow *window);

	public: //private: TODO: TO PRIVATE
		VulkanDevice& _device;
		VulkanSurface& _surface;

		VkSwapchainKHR _swapChain;
		std::vector<VkImage> _swapChainImages;
		VkFormat _swapChainImageFormat;
		VkExtent2D _swapChainExtent;
		std::vector<VkImageView> _swapChainImageViews;
		std::vector<VkFramebuffer> _swapChainFramebuffers;

	};
}