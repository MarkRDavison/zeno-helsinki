#include <helsinki/Renderer/Vulkan/VulkanTexture.hpp>
#include <helsinki/Renderer/Vulkan/VulkanBuffer.hpp>
#include <iostream>
#include <stdexcept>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <cmath>
#include <cassert>

namespace hl
{
	namespace
	{
		VkFormat formatFor(VulkanTextureSampling sampling)
		{
			return sampling == VulkanTextureSampling::SdfUnorm
				? VK_FORMAT_R8G8B8A8_UNORM
				: VK_FORMAT_R8G8B8A8_SRGB;
		}

		uint32_t mipLevelsFor(VulkanTextureSampling sampling, uint32_t width, uint32_t height)
		{
			if (sampling == VulkanTextureSampling::SdfUnorm
				|| sampling == VulkanTextureSampling::PixelArtSrgb)
			{
				return 1;
			}

			return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
		}

		bool generatesMips(VulkanTextureSampling sampling)
		{
			return sampling == VulkanTextureSampling::ColorSrgb;
		}

		VkImageUsageFlags usageFor(VulkanTextureSampling sampling)
		{
			VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			if (sampling == VulkanTextureSampling::ColorSrgb)
			{
				usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			}

			return usage;
		}
	}

	VulkanTexture::VulkanTexture(
		VulkanDevice& device
	) :
		_device(device),
		_image(_device)
	{

	}

	void VulkanTexture::create(
		VulkanCommandPool& commandPool,
		const std::string& filepath,
		VulkanTextureSampling sampling)
	{
		create(commandPool, std::vector<std::string>{ filepath }, sampling);
	}

	void VulkanTexture::create(
		VulkanCommandPool& commandPool,
		const std::vector<std::string>& filepaths,
		VulkanTextureSampling sampling)
	{
		assert(filepaths.size() == 1 || filepaths.size() == 6);
		assert(sampling != VulkanTextureSampling::SdfUnorm || filepaths.size() == 1);

		const VkFormat format = formatFor(sampling);
		int texWidth = 0, texHeight = 0, texChannels = 0;

		for (size_t i = 0; i < filepaths.size(); ++i)
		{
			stbi_uc* pixels = stbi_load(filepaths[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			VkDeviceSize imageSize = texWidth * texHeight * 4;

			if (!pixels)
			{
				throw std::runtime_error("failed to load texture image!");
			}

			if (i == 0)
			{
				_mipLevels = mipLevelsFor(
					sampling,
					static_cast<uint32_t>(texWidth),
					static_cast<uint32_t>(texHeight));
			}

			VulkanBuffer stagingBuffer(_device);

			stagingBuffer.create(
				imageSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			stagingBuffer.mapMemory(pixels);

			stbi_image_free(pixels);

			if (i == 0)
			{
				_image.create(
					texWidth,
					texHeight,
					_mipLevels,
					VK_SAMPLE_COUNT_1_BIT,
					format,
					VK_IMAGE_TILING_OPTIMAL,
					usageFor(sampling),
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					(uint32_t)filepaths.size());

				_image.transitionImageLayout(
					commandPool,
					format,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					_mipLevels);
			}

			_image.copyBufferToImage(
				commandPool,
				stagingBuffer,
				static_cast<uint32_t>(texWidth),
				static_cast<uint32_t>(texHeight),
				(uint32_t)i);

			stagingBuffer.destroy();
		}

		if (generatesMips(sampling))
		{
			_image.generateMipmaps(
				commandPool,
				format,
				texWidth,
				texHeight,
				_mipLevels);
		}
		else
		{
			_image.transitionImageLayout(
				commandPool,
				format,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				_mipLevels);
		}

		_image.createImageView(
			format,
			VK_IMAGE_ASPECT_COLOR_BIT,
			_mipLevels);

		createSampler(sampling);
	}

	void VulkanTexture::create(
		VulkanCommandPool& commandPool,
		const uint8_t* rgba,
		uint32_t width,
		uint32_t height,
		VulkanTextureSampling sampling)
	{
		assert(rgba != nullptr);
		assert(width > 0 && height > 0);

		const VkFormat format = formatFor(sampling);
		_mipLevels = mipLevelsFor(sampling, width, height);

		const VkDeviceSize imageSize = static_cast<VkDeviceSize>(width) * height * 4;
		VulkanBuffer stagingBuffer(_device);
		stagingBuffer.create(
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.mapMemory(rgba);

		_image.create(
			width,
			height,
			_mipLevels,
			VK_SAMPLE_COUNT_1_BIT,
			format,
			VK_IMAGE_TILING_OPTIMAL,
			usageFor(sampling),
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			1);

		_image.transitionImageLayout(
			commandPool,
			format,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			_mipLevels);

		_image.copyBufferToImage(
			commandPool,
			stagingBuffer,
			width,
			height,
			0);

		stagingBuffer.destroy();

		if (generatesMips(sampling))
		{
			_image.generateMipmaps(
				commandPool,
				format,
				static_cast<int32_t>(width),
				static_cast<int32_t>(height),
				_mipLevels);
		}
		else
		{
			_image.transitionImageLayout(
				commandPool,
				format,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				_mipLevels);
		}

		_image.createImageView(
			format,
			VK_IMAGE_ASPECT_COLOR_BIT,
			_mipLevels);

		createSampler(sampling);
	}

	void VulkanTexture::createSampler(VulkanTextureSampling sampling)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(_device.physicalDevice(), &properties); // TODO: CACHE

		const bool sdf = sampling == VulkanTextureSampling::SdfUnorm;
		const bool pixelArt = sampling == VulkanTextureSampling::PixelArtSrgb;
		const bool clampLod = sdf || pixelArt;

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = pixelArt ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
		samplerInfo.minFilter = pixelArt ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
		samplerInfo.addressModeU = clampLod
			? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
			: VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = samplerInfo.addressModeU;
		samplerInfo.addressModeW = samplerInfo.addressModeU;
		samplerInfo.anisotropyEnable = clampLod ? VK_FALSE : VK_TRUE;
		samplerInfo.maxAnisotropy = clampLod ? 1.0f : properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = clampLod
			? VK_SAMPLER_MIPMAP_MODE_NEAREST
			: VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = clampLod ? 0.0f : VK_LOD_CLAMP_NONE;
		samplerInfo.mipLodBias = 0.0f;

		CHECK_VK_RESULT(vkCreateSampler(_device.handle(), &samplerInfo, nullptr, &_sampler));
	}

	void VulkanTexture::destroy()
	{
		vkDestroySampler(_device.handle(), _sampler, nullptr);
		_image.destroy();
	}
}
