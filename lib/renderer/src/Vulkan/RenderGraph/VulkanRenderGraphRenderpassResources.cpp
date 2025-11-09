#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphRenderpassResources.hpp>

namespace hl
{

	VulkanRenderGraphRenderpassResources::VulkanRenderGraphRenderpassResources(
		const std::string& name, 
		VulkanDevice& device, 
		uint32_t imageCount
	) :
		_name(name),
		_imageCount(imageCount),
		_device(device)
	{

	}

	RenderpassAttachment& VulkanRenderGraphRenderpassResources::addAttachment(const std::string& name)
	{
		_attachments.emplace_back();

		auto& added = _attachments.back();
		added.name = name;

		return added;
	}

	void VulkanRenderGraphRenderpassResources::addRenderpass(VkRenderPass renderpass)
	{
		_renderpass = renderpass;
	}
	void VulkanRenderGraphRenderpassResources::destroy()
	{
		vkDestroyRenderPass(_device._device, _renderpass, nullptr);
		_renderpass = VK_NULL_HANDLE;

		for (const auto& a : getAttachments())
		{
			for (auto& i : a.resolveImages)
			{
				i->destroy();
			}

			for (auto& i : a.images)
			{
				i->destroy();
			}
		}
	}

	const std::vector<RenderpassAttachment>& VulkanRenderGraphRenderpassResources::getAttachments() const
	{
		return _attachments;
	}

}