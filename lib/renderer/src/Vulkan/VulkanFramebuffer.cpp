#include <helsinki/Renderer/Vulkan/VulkanFramebuffer.hpp>
#include <iostream>
#include <stdexcept>

namespace hl
{
	VulkanFramebuffer::VulkanFramebuffer(
		VulkanDevice& device
	) :
		_device(device)
	{

	}

	void VulkanFramebuffer::create(
		VulkanRenderpass& renderpass, // TODO: Should the renderpass be part of the constructor?
		std::vector<VkImageView> attachments,
		uint32_t width,
		uint32_t height)
	{
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderpass._renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;

		CHECK_VK_RESULT(vkCreateFramebuffer(_device._device, &framebufferInfo, nullptr, &_framebuffer));
	}

	void VulkanFramebuffer::destroy()
	{
		vkDestroyFramebuffer(_device._device, _framebuffer, nullptr);
	}
}