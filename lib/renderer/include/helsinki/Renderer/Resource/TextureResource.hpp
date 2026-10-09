#pragma once

#include <vulkan/vulkan.h>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Vulkan/VulkanTexture.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>

namespace hl
{
	class TextureResource : public ImageSamplerResource
	{
	public:
		explicit TextureResource(
			const std::string& id,
			ResourceContext& context,
			VulkanTextureSampling sampling = VulkanTextureSampling::ColorSrgb);

		bool Load() override;
		void Unload() override;

		std::pair<VkSampler, VkImageView> getDescriptorInfo(uint32_t frame) const override;

	protected:
		ResourceContext _resourceContext;
		VulkanTexture _texture;
		VulkanTextureSampling _sampling{ VulkanTextureSampling::ColorSrgb };
	};
}