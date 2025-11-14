#pragma once

#include <vulkan/vulkan.h>
#include <helsinki/System/Resource/Resource.hpp>
#include <helsinki/Renderer/Vulkan/VulkanTexture.hpp>

namespace hl
{
	class TextureResource : public Resource
	{
	public:
		explicit TextureResource(const std::string& id, VulkanDevice& device);

	private:
		VulkanDevice _device;
		VulkanTexture _texture;
	};
}