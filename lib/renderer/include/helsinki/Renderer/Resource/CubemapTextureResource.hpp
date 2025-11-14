#pragma once

#include <vulkan/vulkan.h>
#include <helsinki/Renderer/Vulkan/VulkanTexture.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>

namespace hl
{
	class CubemapTextureResource : public TextureResource
	{
	public:
		explicit CubemapTextureResource(const std::string& id, ResourceContext& context);

		bool Load() override;
	};
}