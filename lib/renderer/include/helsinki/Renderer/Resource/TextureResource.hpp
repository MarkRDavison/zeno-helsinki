#pragma once

#include <vulkan/vulkan.h>
#include <helsinki/System/Resource/Resource.hpp>

namespace hl
{
	class TextureResource : public Resource
	{
	public:
		explicit TextureResource(const std::string& id);

	private:
		VkImage image{ VK_NULL_HANDLE };
	};
}