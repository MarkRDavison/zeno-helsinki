#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanImage.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>
#include <cstdint>
#include <vector>
#include <string>

namespace hl
{
	enum class VulkanTextureSampling
	{
		ColorSrgb,
		SdfUnorm,
		PixelArtSrgb
	};

	class VulkanTexture
	{
	public:
		VulkanTexture(VulkanDevice& device);

		void create(
			VulkanCommandPool& commandPool,
			const std::string& filepath,
			VulkanTextureSampling sampling = VulkanTextureSampling::ColorSrgb);
		void create(
			VulkanCommandPool& commandPool,
			const std::vector<std::string>& filepaths,
			VulkanTextureSampling sampling = VulkanTextureSampling::ColorSrgb);
		void create(
			VulkanCommandPool& commandPool,
			const uint8_t* rgba,
			uint32_t width,
			uint32_t height,
			VulkanTextureSampling sampling = VulkanTextureSampling::ColorSrgb);
		void destroy();

	private:
		void createSampler(VulkanTextureSampling sampling);

	public: // private: TODO: to private
		VulkanDevice& _device;
		VulkanImage _image;

		VkSampler _sampler{ VK_NULL_HANDLE };

		uint32_t _mipLevels = 1;
	};
}
