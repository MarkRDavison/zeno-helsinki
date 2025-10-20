#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>
#include <helsinki/Renderer/Vulkan/VulkanBuffer.hpp>

namespace hl
{
	class VulkanImage
	{
	public:
		VulkanImage(VulkanDevice& device);

		void create(
			uint32_t width,
			uint32_t height,
			uint32_t mipMapLevels,
			VkSampleCountFlagBits numSamples,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties);
		void createImageView(
			VkFormat format,
			VkImageAspectFlags aspectFlags,
			uint32_t mipMapLevels);
		void destroy();

		void generateMipmaps(
			VulkanCommandPool& commandPool,
			VkFormat imageFormat, 
			int32_t width, 
			int32_t height, 
			uint32_t mipMapLevels);

		void transitionImageLayout(
			VulkanCommandPool& commandPool,
			VkFormat imageFormat,
			VkImageLayout oldLayout,
			VkImageLayout newLayout,
			uint32_t mipMapLevels);

		void copyBufferToImage(
			VulkanCommandPool& commandPool,
			VulkanBuffer& buffer,
			uint32_t width,
			uint32_t height);

	private:
		VulkanDevice& _device;

	public: // TODO: TO PRIVATE
		VkImage _image;
		VkDeviceMemory _imageMemory;
		VkImageView _imageView;

		uint32_t _width;
		uint32_t _height;
	};
}