#include <helsinki/Renderer/Resource/OffscreenImageResource.hpp>

namespace hl
{
	OffscreenImageResource::OffscreenImageResource(
		const std::string& id
	) :
		ImageSamplerResource(id)
	{
	}

	std::pair<VkSampler, VkImageView> OffscreenImageResource::getDescriptorInfo(uint32_t frame) const
	{
		return { _sampler, _imageViews[frame] };
	}

	void OffscreenImageResource::set(VkSampler sampler, std::vector<VkImageView> imageViews)
	{
		_sampler = sampler;
		_imageViews = imageViews;
	}
}