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
	VulkanTexture::VulkanTexture(
		VulkanDevice& device
	) :
		_device(device),
		_image(_device)
	{

	}

	void VulkanTexture::create(VulkanCommandPool& commandPool, const std::string& filepath)
	{
		create(commandPool, std::vector<std::string>{ filepath });
	}
	void VulkanTexture::create(VulkanCommandPool& commandPool, const std::vector<std::string>& filepaths)
	{
		assert(filepaths.size() == 1 || filepaths.size() == 6);

		int texWidth = 0, texHeight = 0, texChannels = 0;

		for (size_t i = 0; i < filepaths.size(); ++i)
		{
			stbi_uc* pixels = stbi_load(filepaths[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			VkDeviceSize imageSize = texWidth * texHeight * 4;
			_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

			if (!pixels)
			{
				throw std::runtime_error("failed to load texture image!");
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
					VK_FORMAT_R8G8B8A8_SRGB,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					(uint32_t)filepaths.size());

				_image.transitionImageLayout(
					commandPool,
					VK_FORMAT_R8G8B8A8_SRGB,
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
			//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

			stagingBuffer.destroy();
		}

		_image.generateMipmaps(
			commandPool,
			VK_FORMAT_R8G8B8A8_SRGB,
			texWidth,
			texHeight,
			_mipLevels);

		_image.createImageView(
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_ASPECT_COLOR_BIT,
			_mipLevels);

		{

			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(_device._physicalDevice, &properties); // TODO: CACHE

			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.minLod = 0.0f; // Optional
			samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
			samplerInfo.mipLodBias = 0.0f; // Optional

			if (vkCreateSampler(_device._device, &samplerInfo, nullptr, &_sampler) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create texture sampler!");
			}
		}
	}

	void VulkanTexture::destroy()
	{
		vkDestroySampler(_device._device, _sampler, nullptr);
		_image.destroy();
	}
}