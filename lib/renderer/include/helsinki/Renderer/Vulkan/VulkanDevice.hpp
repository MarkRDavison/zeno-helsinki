#pragma once

#include <helsinki/Renderer/Vulkan/VulkanInstance.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSurface.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSwapChainSupportDetails.hpp>
#include <helsinki/Renderer/Vulkan/VulkanQueue.hpp>

namespace hl
{
	class VulkanDevice
	{
	public:
		VulkanDevice(
			VulkanInstance& instance,
			VulkanSurface& surface);
		void create();
		void destroy();

	private:
		void pickPhysicalDevice();
		void createLogicalDevice();

		static bool isDeviceSuitable(VkPhysicalDevice d, VkSurfaceKHR s);
		static bool checkDeviceExtensionSupport(VkPhysicalDevice p);
		static VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice p);

		// TODO: Split logical and physical???
	public: // private: TODO to private
		VulkanInstance& _instance;
		VulkanSurface& _surface;
		VkPhysicalDevice _physicalDevice{ VK_NULL_HANDLE };
		VkDevice _device{ VK_NULL_HANDLE };
		VulkanQueue _graphicsQueue;
		VulkanQueue _presentQueue;
		VkSampleCountFlagBits _msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	};
}