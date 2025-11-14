#include <helsinki/Renderer/Resource/CubemapTextureResource.hpp>
#include <format>

namespace hl
{
	CubemapTextureResource::CubemapTextureResource(
		const std::string& id,
		ResourceContext& context
	) :
		TextureResource(id, context)
	{
	}

	bool CubemapTextureResource::Load()
	{
		_texture.create(
			*_resourceContext.pool,
			{
				std::format("{}/data/textures/{}-right.png", _resourceContext.rootPath, GetId()),
				std::format("{}/data/textures/{}-left.png", _resourceContext.rootPath, GetId()),
				std::format("{}/data/textures/{}-top.png", _resourceContext.rootPath, GetId()),
				std::format("{}/data/textures/{}-bottom.png", _resourceContext.rootPath, GetId()),
				std::format("{}/data/textures/{}-front.png", _resourceContext.rootPath, GetId()),
				std::format("{}/data/textures/{}-back.png", _resourceContext.rootPath, GetId())
			});

		return Resource::Load();
	}
}