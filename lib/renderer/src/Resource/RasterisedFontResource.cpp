#include <helsinki/Renderer/Resource/RasterisedFontResource.hpp>

namespace hl
{

	RasterisedFontResource::RasterisedFontResource(
		const std::string& id,
		ResourceContext& context
	) :
		FontResource(
			id,
			context,
			FontType::Rasterised
		)
	{

	}
}