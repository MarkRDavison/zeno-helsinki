#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>

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

	private:
		VulkanDevice& _device;

	public: // TODO: TO PRIVATE
		VkImage _image;
		VkDeviceMemory _imageMemory;
		VkImageView _imageView;
	};
}