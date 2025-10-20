#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanRenderpass.hpp>

namespace hl
{

	class VulkanFramebuffer
	{
	public:
		VulkanFramebuffer(VulkanDevice& device);

		void create(
			VulkanRenderpass& renderpass,
			std::vector<VkImageView> attachments, 
			uint32_t width, 
			uint32_t height);
		void destroy();

	public: // private: TODO: to private
		VulkanDevice& _device;
		VkFramebuffer _framebuffer{ VK_NULL_HANDLE };
	};

}