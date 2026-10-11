#pragma once

#include <helsinki/Renderer/Export.hpp>

#include <helsinki/Renderer/Resource/FontResource.hpp>

namespace hl
{

	class HELSINKI_RENDERER_EXPORT RasterisedFontResource : public FontResource
	{
	public:
		explicit RasterisedFontResource(
			const std::string& id,
			ResourceContext& context);

	private:

	};

}