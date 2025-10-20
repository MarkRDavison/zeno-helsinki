#include <helsinki/Renderer/Vulkan/VulkanSurface.hpp>
#include <stdexcept>
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace hl
{
	VulkanSurface::VulkanSurface(VulkanInstance& instance) : instance(instance){}

	void VulkanSurface::create(GLFWwindow* window)
	{
		if (glfwCreateWindowSurface(instance._instance, window, nullptr, &surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void VulkanSurface::destroy()
	{
		vkDestroySurfaceKHR(instance._instance, surface, nullptr);
	}

}