#include <helsinki/Renderer/Vulkan/VulkanImage.hpp>
#include <stdexcept>

namespace hl
{
    VulkanImage::VulkanImage(
        VulkanDevice& device
    ) :
        _device(device),
        _image(VK_NULL_HANDLE),
        _imageMemory(VK_NULL_HANDLE),
        _imageView(VK_NULL_HANDLE)
    {

    }

    void VulkanImage::create(
        uint32_t width,
        uint32_t height,
        uint32_t mipMapLevels,
        VkSampleCountFlagBits numSamples,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties)
    {
        VkImageCreateInfo imageInfo{};

        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipMapLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = numSamples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(_device._device, &imageInfo, nullptr, &_image) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(_device._device, _image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = _device.findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(_device._device, &allocInfo, nullptr, &_imageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(_device._device, _image, _imageMemory, 0);
    }

    // TODO: Make private?
    void VulkanImage::createImageView(
        VkFormat format,
        VkImageAspectFlags aspectFlags,
        uint32_t mipMapLevels)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = _image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipMapLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(_device._device, &viewInfo, nullptr, &_imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image view!");
        }
    }

    void VulkanImage::destroy()
    {
        if (_imageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(_device._device, _imageView, nullptr);
        }

        vkDestroyImage(_device._device, _image, nullptr);
        vkFreeMemory(_device._device, _imageMemory, nullptr);
    }
}