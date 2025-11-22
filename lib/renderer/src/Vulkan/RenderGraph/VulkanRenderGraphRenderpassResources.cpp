#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphRenderpassResources.hpp>
#include <cassert>

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

	void VulkanRenderGraphRenderpassResources::startPipelineGroup()
	{
		assert(!_pipelineGroupOpen);
		_pipelineGroupOpen = true;
		_pipelineGroups.push_back({});
	}
	void VulkanRenderGraphRenderpassResources::endPipelineGroup()
	{
		assert(_pipelineGroupOpen);
		_pipelineGroupOpen = false;
	}
	VulkanRenderGraphPipelineResources& VulkanRenderGraphRenderpassResources::addPipeline(const std::string& name)
	{
		assert(_pipelineGroupOpen);
		auto pipeline = new VulkanRenderGraphPipelineResources(name, _device);

		_pipelineGroups.back().push_back(pipeline);

		return *pipeline;
	}

	void VulkanRenderGraphRenderpassResources::addDescriptorPool(VkDescriptorPool descriptorPool)
	{
		_descriptorPool = descriptorPool;
	}

	void VulkanRenderGraphRenderpassResources::destroy()
	{
		vkDestroyDescriptorPool(_device._device, _descriptorPool, nullptr);

		for (auto& pg : _pipelineGroups)
		{
			for (auto& p : pg)
			{
				p->destroy();
				delete p;
			}
		}

		_pipelineGroups.clear();

		for (auto& fb : _framebuffers)
		{
			vkDestroyFramebuffer(_device._device, fb, nullptr);
		}

		_framebuffers.clear();

		vkDestroyRenderPass(_device._device, _renderpass, nullptr);
		_renderpass = VK_NULL_HANDLE;

		for (auto& a : getAttachments())
		{
			if (a.sampler != VK_NULL_HANDLE)
			{
				vkDestroySampler(_device._device, a.sampler, nullptr);
				a.sampler = VK_NULL_HANDLE;
			}

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

	void VulkanRenderGraphRenderpassResources::recreate(
		const RenderpassInfo& info,
		uint32_t width, 
		uint32_t height,
		const std::vector<VkImageView>& swapChainImageViews,
		uint32_t imageCount,
		bool isLastRenderpass)
	{
		// Destroy
		for (auto& fb : _framebuffers)
		{
			vkDestroyFramebuffer(_device._device, fb, nullptr);
		}

		_framebuffers.clear();

		for (auto& a : getAttachments())
		{
			if (a.sampler != VK_NULL_HANDLE)
			{
				vkDestroySampler(_device._device, a.sampler, nullptr);
				a.sampler = VK_NULL_HANDLE;
			}

			for (auto& i : a.resolveImages)
			{
				i->destroy();
				delete i;
			}

			for (auto& i : a.images)
			{
				i->destroy();
				delete i;
			}
		}

		_attachments.clear();

		// Create
		RenderGraph::createImages(
			_device,
			this,
			info,
			width,
			height,
			imageCount, 
			isLastRenderpass);

		RenderGraph::createFrameBuffers(
			_device,
			this,
			info,
			width,
			height,
			swapChainImageViews,
			imageCount,
			isLastRenderpass);
	}

	const std::vector<RenderpassAttachment>& VulkanRenderGraphRenderpassResources::getAttachments() const
	{
		return _attachments;
	}
	std::vector<RenderpassAttachment>& VulkanRenderGraphRenderpassResources::getAttachments()
	{
		return _attachments;
	}
	const std::vector<std::vector<VulkanRenderGraphPipelineResources*>>& VulkanRenderGraphRenderpassResources::getPipelineGroupss() const
	{
		return _pipelineGroups;
	}

	const VkRenderPass VulkanRenderGraphRenderpassResources::getRenderPass() const
	{
		return _renderpass;
	}

	const VkFramebuffer VulkanRenderGraphRenderpassResources::getFramebuffer(uint32_t imageIndex)
	{
		return _framebuffers[imageIndex];
	}

	std::vector<VkClearValue> VulkanRenderGraphRenderpassResources::getClearValues() const
	{
		return _clearValues;
	}
	void VulkanRenderGraphRenderpassResources::setClearValues(const std::vector<VkClearValue>& clearValues)
	{
		_clearValues = std::vector<VkClearValue>(clearValues);
	}
	VkExtent2D VulkanRenderGraphRenderpassResources::getExtent() const
	{
		return _extent;
	}
	void VulkanRenderGraphRenderpassResources::setExtent(VkExtent2D extent)
	{
		_extent = extent;
	}
}