#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Renderer/Resource/MaterialSystem.hpp>
#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <cstdint>
#include <format>

namespace hl
{
	TextureResource::TextureResource(
		const std::string& id, 
		ResourceContext& context
	) : 
		ImageSamplerResource(id),
		_resourceContext(context),
		_texture(*context.device)
	{
	}

	bool TextureResource::Load()
	{
		if (GetId() == MaterialSystem::FallbackTextureName)
		{
			constexpr uint8_t kWhitePixel[] = { 255, 255, 255, 255 };
			_texture.create(*_resourceContext.pool, kWhitePixel, 1, 1);
			return Resource::Load();
		}

		auto sampling = VulkanTextureSampling::ColorSrgb;
		if (_resourceContext.resourceManager != nullptr
			&& _resourceContext.resourceManager->HasResource<FontResource>(GetId()))
		{
			const auto* font = _resourceContext.resourceManager->GetResource<FontResource>(GetId());
			if (font != nullptr && font->getFontType() == FontType::SignedDistanceField)
			{
				sampling = VulkanTextureSampling::SdfUnorm;
			}
		}

		std::string path = std::format("{}/data/textures/{}.png", _resourceContext.rootPath, GetId());

		_texture.create(
			*_resourceContext.pool, 
			{ 
				path 
			},
			sampling);

		return Resource::Load();
	}

	void TextureResource::Unload()
	{
		if (IsLoaded())
		{
			_texture.destroy();
			Resource::Unload();
		}
	}

	std::pair<VkSampler, VkImageView> TextureResource::getDescriptorInfo(uint32_t /*frame*/) const
	{
		return { _texture._sampler, _texture._image._imageView };
	}
}
