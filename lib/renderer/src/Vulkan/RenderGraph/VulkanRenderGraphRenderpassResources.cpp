#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphRenderpassResources.hpp>

namespace hl
{

	VulkanRenderGraphRenderpassResources::VulkanRenderGraphRenderpassResources(
		const std::string& name, 
		VulkanDevice& device, 
		uint32_t imageCount
	) :
		Name(name),
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

	void VulkanRenderGraphRenderpassResources::addFramebuffer(VkFramebuffer framebuffer)
	{
		_framebuffers.push_back(framebuffer);
	}
	VulkanRenderGraphPipelineResources& VulkanRenderGraphRenderpassResources::addPipeline(const std::string& name)
	{
		auto pipeline = new VulkanRenderGraphPipelineResources(name, _device);

		_pipelines.push_back(pipeline);

		return *pipeline;
	}

	void VulkanRenderGraphRenderpassResources::destroy()
	{
		for (auto& p : _pipelines)
		{
			p->destroy();
			delete p;
		}

		_pipelines.clear();

		for (auto& fb : _framebuffers)
		{
			vkDestroyFramebuffer(_device._device, fb, nullptr);
		}

		_framebuffers.clear();

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

	const VkRenderPass VulkanRenderGraphRenderpassResources::getRenderPass() const
	{
		return _renderpass;
	}
}