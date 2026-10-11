#pragma once

#include <helsinki/Renderer/Export.hpp>

#include <vulkan/vulkan.h>
#include <helsinki/Renderer/Vulkan/VulkanTexture.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>

namespace hl
{
	class HELSINKI_RENDERER_EXPORT CubemapTextureResource : public TextureResource
	{
	public:
		explicit CubemapTextureResource(const std::string& id, ResourceContext& context);

		bool Load() override;
	};
}