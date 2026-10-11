#pragma once

#include <helsinki/Renderer/Export.hpp>

#include <helsinki/Renderer/Vulkan/VulkanInstance.hpp>

namespace hl
{
	class HELSINKI_RENDERER_EXPORT VulkanSurface
	{
	public:
		VulkanSurface(VulkanInstance& instance);
		void create(VkSurfaceKHR surface);
		void destroy();

		VkSurfaceKHR handle() const { return _surface; }

	private:
		VulkanInstance& _instance;
		VkSurfaceKHR _surface{ VK_NULL_HANDLE };
	};
}
