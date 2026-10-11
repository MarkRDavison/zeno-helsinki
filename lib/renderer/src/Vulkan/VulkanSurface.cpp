#include <helsinki/Renderer/Vulkan/VulkanSurface.hpp>

namespace hl
{
	VulkanSurface::VulkanSurface(VulkanInstance& instance) : _instance(instance){}

	void VulkanSurface::create(VkSurfaceKHR surface)
	{
		_surface = surface;
	}

	void VulkanSurface::destroy()
	{
		vkDestroySurfaceKHR(_instance.handle(), _surface, nullptr);
	}

}
