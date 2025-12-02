#pragma once

#include <helsinki/Renderer/Resource/FontResource.hpp>

namespace hl
{

	class RasterisedFontResource : public FontResource
	{
	public:
		explicit RasterisedFontResource(
			const std::string& id,
			ResourceContext& context);

	private:

	};

}