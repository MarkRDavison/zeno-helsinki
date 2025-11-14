#pragma once

#include <vulkan/vulkan.h>
#include <helsinki/System/Resource/Resource.hpp>

namespace hl
{

	class ImageSamplerResource : public Resource
	{
	public:
		explicit ImageSamplerResource(const std::string& id) : Resource(id) {}

		virtual std::pair<VkSampler, VkImageView> getDescriptorInfo(uint32_t /*frame*/) const
		{
			return { VK_NULL_HANDLE, VK_NULL_HANDLE };
		}
	};

}