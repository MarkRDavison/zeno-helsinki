#pragma once

#include <helsinki/Renderer/Export.hpp>

#include <vulkan/vulkan.h>
#include <vector>

namespace hl
{

	class HELSINKI_RENDERER_EXPORT VulkanInstance
	{
	public:
		void create(const char *title, const std::vector<const char*>& windowExtensions);
		void destroy();

		VkInstance handle() const { return _instance; }

	private:
		static bool checkValidationLayerSupport();
		static std::vector<const char*> getRequiredExtensions(const std::vector<const char*>& windowExtensions);
		static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		static void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT& debugMessenger);

		VkInstance _instance{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT _debugMessenger{ VK_NULL_HANDLE };
	};

}
