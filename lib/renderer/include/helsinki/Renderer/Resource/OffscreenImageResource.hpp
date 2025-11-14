#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <helsinki/Renderer/Vulkan/VulkanTexture.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>

namespace hl
{
	class OffscreenImageResource : public ImageSamplerResource
	{
	public:
		explicit OffscreenImageResource(const std::string& id);

		std::pair<VkSampler, VkImageView> getDescriptorInfo(uint32_t frame) const override;

		void set(VkSampler sampler, std::vector<VkImageView> imageViews);

	private:
		VkSampler _sampler{ VK_NULL_HANDLE };
		std::vector<VkImageView> _imageViews;
	};
}