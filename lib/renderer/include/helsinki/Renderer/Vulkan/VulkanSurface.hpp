#pragma once

#include <helsinki/Renderer/Vulkan/VulkanInstance.hpp>

struct GLFWwindow;

namespace hl
{
	class VulkanSurface
	{
	public:
		VulkanSurface(VulkanInstance& instance);
		void create(GLFWwindow *window);
		void destroy();

	public: // TODO: MAKE PRIVATE
		VulkanInstance& _instance;
		VkSurfaceKHR _surface{ VK_NULL_HANDLE };
		GLFWwindow* _window{ nullptr };
	};
}