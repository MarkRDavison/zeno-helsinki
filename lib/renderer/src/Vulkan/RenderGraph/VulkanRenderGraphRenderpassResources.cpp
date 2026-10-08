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

	void VulkanRenderGraphRenderpassResources::addPipelineGroupDescriptorSetLayout(VkDescriptorSetLayout layout)
	{
		_pipelineGroupDescriptorSetLayouts.push_back(layout);
	}

	void VulkanRenderGraphRenderpassResources::destroy()
	{
		vkDestroyDescriptorPool(_device.handle(), _descriptorPool, nullptr);

		for (auto& pg : _pipelineGroups)
		{
			for (auto& p : pg)
			{
				p->destroy();
				delete p;
			}
		}

		_pipelineGroups.clear();

		for (auto layout : _pipelineGroupDescriptorSetLayouts)
		{
			vkDestroyDescriptorSetLayout(_device.handle(), layout, nullptr);
		}

		_pipelineGroupDescriptorSetLayouts.clear();

		for (auto& a : getAttachments())
		{
			if (a.sampler != VK_NULL_HANDLE)
			{
				vkDestroySampler(_device.handle(), a.sampler, nullptr);
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
		uint32_t imageCount,
		bool isLastRenderpass)
	{
		for (auto& a : getAttachments())
		{
			if (a.sampler != VK_NULL_HANDLE)
			{
				vkDestroySampler(_device.handle(), a.sampler, nullptr);
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

		RenderGraph::createImages(
			_device,
			this,
			info,
			width,
			height,
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
	const std::vector<std::vector<VulkanRenderGraphPipelineResources*>>& VulkanRenderGraphRenderpassResources::getPipelineGroups() const
	{
		return _pipelineGroups;
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

	void VulkanRenderGraphRenderpassResources::setRenderingState(
		const std::vector<VkFormat>& colorFormats,
		VkFormat depthFormat,
		VkSampleCountFlagBits rasterizationSamples,
		bool writesToSwapchain)
	{
		_colorFormats = colorFormats;
		_depthFormat = depthFormat;
		_rasterizationSamples = rasterizationSamples;
		_writesToSwapchain = writesToSwapchain;
	}

	const std::vector<VkFormat>& VulkanRenderGraphRenderpassResources::getColorFormats() const
	{
		return _colorFormats;
	}

	VkFormat VulkanRenderGraphRenderpassResources::getDepthFormat() const
	{
		return _depthFormat;
	}

	VkSampleCountFlagBits VulkanRenderGraphRenderpassResources::getRasterizationSamples() const
	{
		return _rasterizationSamples;
	}

	bool VulkanRenderGraphRenderpassResources::writesToSwapchain() const
	{
		return _writesToSwapchain;
	}

	void VulkanRenderGraphRenderpassResources::setIsCompute(bool isCompute)
	{
		_isCompute = isCompute;
	}

	bool VulkanRenderGraphRenderpassResources::isCompute() const
	{
		return _isCompute;
	}

	bool VulkanRenderGraphRenderpassResources::usesMultiSampling() const
	{
		return _rasterizationSamples != VK_SAMPLE_COUNT_1_BIT;
	}
}
