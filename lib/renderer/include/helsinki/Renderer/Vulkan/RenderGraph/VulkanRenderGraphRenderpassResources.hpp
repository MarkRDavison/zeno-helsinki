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
		VulkanRenderGraphPipelineResources& addPipeline(const std::string& name);
		void addDescriptorPool(VkDescriptorPool descriptorPool);
		void destroy();

		const std::vector<RenderpassAttachment>& getAttachments() const;
		std::vector<RenderpassAttachment>& getAttachments();
		const std::vector<VulkanRenderGraphPipelineResources*>& getPipelines() const;

		const VkRenderPass getRenderPass() const;

		const std::string Name;

	private:
		const uint32_t _imageCount;
		VulkanDevice& _device;
		std::vector<RenderpassAttachment> _attachments;
		VkRenderPass _renderpass{ VK_NULL_HANDLE };
		std::vector<VkFramebuffer> _framebuffers;
		std::vector<VulkanRenderGraphPipelineResources*> _pipelines;
		VkDescriptorPool _descriptorPool{ VK_NULL_HANDLE };
	};

}