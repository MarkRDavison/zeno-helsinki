#include <helsinki/Renderer/Vulkan/VulkanSurface.hpp>
#include <stdexcept>
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace hl
{
	VulkanSurface::VulkanSurface(VulkanInstance& instance) : _instance(instance){}

	void VulkanSurface::create(GLFWwindow* window)
	{
		_window = window;

		if (glfwCreateWindowSurface(_instance._instance, window, nullptr, &_surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void VulkanSurface::destroy()
	{
		vkDestroySurfaceKHR(_instance._instance, _surface, nullptr);
	}

}