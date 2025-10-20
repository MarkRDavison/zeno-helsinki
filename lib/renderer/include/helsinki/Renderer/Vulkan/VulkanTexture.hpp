#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanImage.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>
#include <string>

namespace hl
{
	class VulkanTexture
	{
	public:
		VulkanTexture(VulkanDevice& device);

		void create(VulkanCommandPool& commandPool, const std::string& filepath);
		void destroy();

	public: // private: TODO: to private
		VulkanDevice& _device;
		VulkanImage _image;

		VkSampler _sampler{ VK_NULL_HANDLE };

		uint32_t _mipLevels = 1;
	};
}