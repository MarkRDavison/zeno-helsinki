#pragma once

#include <string>
#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphPipelineResources.hpp>

namespace hl
{

	class VulkanRenderGraphRenderpassResources
	{
	public:
		VulkanRenderGraphRenderpassResources(const std::string& name, VulkanDevice& device, uint32_t imageCount);

		RenderpassAttachment& addAttachment(const std::string& name);
		void startPipelineGroup();
		void endPipelineGroup();
		VulkanRenderGraphPipelineResources& addPipeline(const std::string& name);
		void addDescriptorPool(VkDescriptorPool descriptorPool);
		void addPipelineGroupDescriptorSetLayout(VkDescriptorSetLayout layout);
		void destroy();
		void recreate(
			const RenderpassInfo& info,
			uint32_t width,
			uint32_t height,
			uint32_t imageCount,
			bool isLastRenderpass);

		const std::vector<RenderpassAttachment>& getAttachments() const;
		std::vector<RenderpassAttachment>& getAttachments();
		const std::vector<std::vector<VulkanRenderGraphPipelineResources*>>& getPipelineGroups() const;

		std::vector<VkClearValue> getClearValues() const;
		void setClearValues(const std::vector<VkClearValue>& clearValues);
		VkExtent2D getExtent() const;
		void setExtent(VkExtent2D extent);

		void setRenderingState(
			const std::vector<VkFormat>& colorFormats,
			VkFormat depthFormat,
			VkSampleCountFlagBits rasterizationSamples,
			bool writesToSwapchain);
		const std::vector<VkFormat>& getColorFormats() const;
		VkFormat getDepthFormat() const;
		VkSampleCountFlagBits getRasterizationSamples() const;
		bool writesToSwapchain() const;
		bool usesMultiSampling() const;
		void setIsCompute(bool isCompute);
		bool isCompute() const;

		const std::string Name;

	private:
		const uint32_t _imageCount;
		VulkanDevice& _device;
		std::vector<RenderpassAttachment> _attachments;
		std::vector<std::vector<VulkanRenderGraphPipelineResources*>> _pipelineGroups;
		std::vector<VkDescriptorSetLayout> _pipelineGroupDescriptorSetLayouts;
		VkDescriptorPool _descriptorPool{ VK_NULL_HANDLE };
		std::vector<VkClearValue> _clearValues;
		VkExtent2D _extent{};
		bool _pipelineGroupOpen{ false };
		std::vector<VkFormat> _colorFormats;
		VkFormat _depthFormat{ VK_FORMAT_UNDEFINED };
		VkSampleCountFlagBits _rasterizationSamples{ VK_SAMPLE_COUNT_1_BIT };
		bool _writesToSwapchain{ false };
		bool _isCompute{ false };
	};

}
