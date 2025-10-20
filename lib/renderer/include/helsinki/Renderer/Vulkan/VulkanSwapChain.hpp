#pragma once

#include <helsinki/Renderer/Vulkan/VulkanSwapChainSupportDetails.hpp>
#include <vulkan/vulkan.h>

namespace hl
{
	class VulkanSwapChain
	{
	public:
		void create();
		void destroy();

		static VulkanSwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice p, VkSurfaceKHR s);

	public: //private: TODO: TO PRIVATE
		VkSwapchainKHR _swapChain;
		std::vector<VkImage> _swapChainImages;
		VkFormat _swapChainImageFormat;
		VkExtent2D _swapChainExtent;
		std::vector<VkImageView> _swapChainImageViews;
		std::vector<VkFramebuffer> _swapChainFramebuffers;

	};
}