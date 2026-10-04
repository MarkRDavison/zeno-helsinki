#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace hl
{

	class VulkanInstance
	{
	public:
		void create(const char *title);
		void destroy();

		VkInstance handle() const { return _instance; }

	private:
		static bool checkValidationLayerSupport();
		static std::vector<const char*> getRequiredExtensions();
		static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		static void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT& debugMessenger);

		VkInstance _instance{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT _debugMessenger{ VK_NULL_HANDLE };
	};

}
