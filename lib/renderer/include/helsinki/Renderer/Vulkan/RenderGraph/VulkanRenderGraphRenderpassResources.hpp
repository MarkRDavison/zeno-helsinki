#pragma once

#include <string>
#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanRenderpass.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>

namespace hl
{

	class VulkanRenderGraphRenderpassResources
	{
	public:
		VulkanRenderGraphRenderpassResources(const std::string& name, VulkanDevice& device, uint32_t imageCount);

		RenderpassAttachment& addAttachment(const std::string& name);
		void addRenderpass(VkRenderPass renderpass);
		void destroy();

		const std::vector<RenderpassAttachment>& getAttachments() const;

	private:
		const std::string _name;
		const uint32_t _imageCount;
		VulkanDevice& _device;
		std::vector<RenderpassAttachment> _attachments;
		VkRenderPass _renderpass{ VK_NULL_HANDLE };
	};

}