#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <format>

namespace hl
{
	TextureResource::TextureResource(
		const std::string& id, 
		ResourceContext& context
	) : 
		Resource(id),
		_resourceContext(context),
		_texture(*context.device)
	{
	}

	bool TextureResource::Load()
	{		
		std::string path = std::format("{}/data/textures/{}.png", _resourceContext.rootPath, GetId());

		_texture.create(
			*_resourceContext.pool, 
			{ 
				path 
			});

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

	std::pair<VkSampler, VkImageView> TextureResource::getDescriptorInfo() const
	{
		return { _texture._sampler, _texture._image._imageView };
	}
}