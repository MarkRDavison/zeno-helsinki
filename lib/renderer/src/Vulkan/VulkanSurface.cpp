#include <helsinki/Renderer/Vulkan/VulkanSurface.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
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

		CHECK_VK_RESULT(glfwCreateWindowSurface(_instance._instance, window, nullptr, &_surface));
	}

	void VulkanSurface::destroy()
	{
		vkDestroySurfaceKHR(_instance._instance, _surface, nullptr);
	}

}