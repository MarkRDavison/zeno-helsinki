#pragma once

#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/Renderer/Vulkan/VulkanInstance.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSurface.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSwapChainSupportDetails.hpp>
#include <helsinki/Renderer/Vulkan/VulkanQueue.hpp>
#include <iostream>
#include <unordered_map>

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
		void waitIdle();

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

		inline void setDebugName(uint64_t handle, VkObjectType type, const char* name)
		{
			VkDebugUtilsObjectNameInfoEXT nameInfo{};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			nameInfo.objectType = type;
			nameInfo.objectHandle = handle;
			nameInfo.pObjectName = name;

			_handleToNameMap.insert({ handle, std::string(name) });

			auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(_device, "vkSetDebugUtilsObjectNameEXT");
			if (func)
			{
				func(_device, &nameInfo);
			}
		}

		inline std::string getNameFromHandle(uint64_t handle)
		{
			return _handleToNameMap.at(handle);
		}

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
		std::unordered_map<uint64_t, std::string> _handleToNameMap;
	};
}