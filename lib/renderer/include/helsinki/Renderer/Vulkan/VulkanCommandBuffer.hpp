#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>

namespace hl
{
	class VulkanCommandBuffer
	{
	public:
		VulkanCommandBuffer(VulkanDevice& device);

		static void pipelineBarrier2(VulkanDevice& device, VkCommandBuffer commandBuffer, const VkDependencyInfo& dependencyInfo);
		static void undefinedToColorAttachment(VulkanDevice& device, VkCommandBuffer commandBuffer, VkImage image);
		static void undefinedToDepthAttachment(VulkanDevice& device, VkCommandBuffer commandBuffer, VkImage image);
		static void colorAttachmentToSampled(VulkanDevice& device, VkCommandBuffer commandBuffer, VkImage image);
		static void depthAttachmentToSampled(VulkanDevice& device, VkCommandBuffer commandBuffer, VkImage image);
		static void colorAttachmentToPresent(VulkanDevice& device, VkCommandBuffer commandBuffer, VkImage image);
		static void storageWriteToVertexRead(VulkanDevice& device, VkCommandBuffer commandBuffer, VkBuffer buffer);

	public: // private: TODO: to private
		VulkanDevice& _device;

		VkCommandBuffer _commandBuffer{ VK_NULL_HANDLE };
	};
}
