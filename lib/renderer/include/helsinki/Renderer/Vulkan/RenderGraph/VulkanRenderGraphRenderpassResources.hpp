#pragma once

#include <string>
#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanRenderpass.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphPipelineResources.hpp>

namespace hl
{

	class VulkanRenderGraphRenderpassResources
	{
	public:
		VulkanRenderGraphRenderpassResources(const std::string& name, VulkanDevice& device, uint32_t imageCount);

		RenderpassAttachment& addAttachment(const std::string& name);
		void addRenderpass(VkRenderPass renderpass);
		void addFramebuffer(VkFramebuffer framebuffer);
		void startPipelineGroup();
		void endPipelineGroup();
		VulkanRenderGraphPipelineResources& addPipeline(const std::string& name);
		void addDescriptorPool(VkDescriptorPool descriptorPool);
		void destroy();
		void recreate(
			const RenderpassInfo& info,
			uint32_t width,
			uint32_t height,
			const std::vector<VkImageView>& swapChainImageViews,
			uint32_t imageCount,
			bool isLastRenderpass);

		const std::vector<RenderpassAttachment>& getAttachments() const;
		std::vector<RenderpassAttachment>& getAttachments();
		const std::vector<std::vector<VulkanRenderGraphPipelineResources*>>& getPipelineGroups() const;

		const VkRenderPass getRenderPass() const;
		const VkFramebuffer getFramebuffer(uint32_t imageIndex);

		std::vector<VkClearValue> getClearValues() const;
		void setClearValues(const std::vector<VkClearValue>& clearValues);
		VkExtent2D getExtent() const;
		void setExtent(VkExtent2D extent);

		const std::string Name;

	private:
		const uint32_t _imageCount;
		VulkanDevice& _device;
		std::vector<RenderpassAttachment> _attachments;
		VkRenderPass _renderpass{ VK_NULL_HANDLE };
		std::vector<VkFramebuffer> _framebuffers;
		std::vector<std::vector<VulkanRenderGraphPipelineResources*>> _pipelineGroups;
		VkDescriptorPool _descriptorPool{ VK_NULL_HANDLE };
		std::vector<VkClearValue> _clearValues;
		VkExtent2D _extent{};
		bool _pipelineGroupOpen{ false };
	};

}