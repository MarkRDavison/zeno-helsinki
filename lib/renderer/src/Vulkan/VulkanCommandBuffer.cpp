#include <helsinki/Renderer/Vulkan/VulkanCommandBuffer.hpp>
#include <cassert>

namespace hl
{
	VulkanCommandBuffer::VulkanCommandBuffer(
		VulkanDevice& device
	) : _device(device)
	{

	}

	static VkImageMemoryBarrier2 makeImageBarrier(
		VkImage image,
		VkPipelineStageFlags2 srcStage,
		VkPipelineStageFlags2 dstStage,
		VkAccessFlags2 srcAccess,
		VkAccessFlags2 dstAccess,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkImageAspectFlags aspect)
	{
		assert(image != VK_NULL_HANDLE);
		assert(aspect != 0);

		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.srcStageMask = srcStage;
		barrier.srcAccessMask = srcAccess;
		barrier.dstStageMask = dstStage;
		barrier.dstAccessMask = dstAccess;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = aspect;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		return barrier;
	}

	static void submitImageBarrier(
		VulkanDevice& device,
		VkCommandBuffer commandBuffer,
		const VkImageMemoryBarrier2& barrier)
	{
		assert(commandBuffer != VK_NULL_HANDLE);
		VkDependencyInfo dependencyInfo{};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.imageMemoryBarrierCount = 1;
		dependencyInfo.pImageMemoryBarriers = &barrier;
		VulkanCommandBuffer::pipelineBarrier2(device, commandBuffer, dependencyInfo);
	}

	void VulkanCommandBuffer::storageWriteToVertexRead(
		VulkanDevice& device,
		VkCommandBuffer commandBuffer,
		VkBuffer buffer)
	{
		assert(commandBuffer != VK_NULL_HANDLE);
		assert(buffer != VK_NULL_HANDLE);

		VkBufferMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		barrier.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.buffer = buffer;
		barrier.offset = 0;
		barrier.size = VK_WHOLE_SIZE;

		VkDependencyInfo dependencyInfo{};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.bufferMemoryBarrierCount = 1;
		dependencyInfo.pBufferMemoryBarriers = &barrier;
		VulkanCommandBuffer::pipelineBarrier2(device, commandBuffer, dependencyInfo);
	}

	void VulkanCommandBuffer::pipelineBarrier2(
		VulkanDevice& device,
		VkCommandBuffer commandBuffer,
		const VkDependencyInfo& dependencyInfo)
	{
		assert(commandBuffer != VK_NULL_HANDLE);
		assert(device.cmdPipelineBarrier2() != nullptr);
		device.cmdPipelineBarrier2()(commandBuffer, &dependencyInfo);
	}

	void VulkanCommandBuffer::undefinedToColorAttachment(VulkanDevice& device, VkCommandBuffer commandBuffer, VkImage image)
	{
		submitImageBarrier(
			device,
			commandBuffer,
			makeImageBarrier(
				image,
				VK_PIPELINE_STAGE_2_NONE,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_2_NONE,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT));
	}

	void VulkanCommandBuffer::undefinedToDepthAttachment(VulkanDevice& device, VkCommandBuffer commandBuffer, VkImage image)
	{
		submitImageBarrier(
			device,
			commandBuffer,
			makeImageBarrier(
				image,
				VK_PIPELINE_STAGE_2_NONE,
				VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
				VK_ACCESS_2_NONE,
				VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				VK_IMAGE_ASPECT_DEPTH_BIT));
	}

	void VulkanCommandBuffer::colorAttachmentToSampled(VulkanDevice& device, VkCommandBuffer commandBuffer, VkImage image)
	{
		submitImageBarrier(
			device,
			commandBuffer,
			makeImageBarrier(
				image,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT));
	}

	void VulkanCommandBuffer::depthAttachmentToSampled(VulkanDevice& device, VkCommandBuffer commandBuffer, VkImage image)
	{
		submitImageBarrier(
			device,
			commandBuffer,
			makeImageBarrier(
				image,
				VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
				VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
				VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_ASPECT_DEPTH_BIT));
	}

	void VulkanCommandBuffer::colorAttachmentToPresent(VulkanDevice& device, VkCommandBuffer commandBuffer, VkImage image)
	{
		submitImageBarrier(
			device,
			commandBuffer,
			makeImageBarrier(
				image,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_2_NONE,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_2_NONE,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_IMAGE_ASPECT_COLOR_BIT));
	}
}
