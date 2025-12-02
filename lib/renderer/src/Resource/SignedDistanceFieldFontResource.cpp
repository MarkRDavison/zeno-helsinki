#include <helsinki/Renderer/Resource/SignedDistanceFieldFontResource.hpp>

namespace hl
{

	SignedDistanceFieldFontResource::SignedDistanceFieldFontResource(
		const std::string& id,
		ResourceContext& context
	) :
		FontResource(
			id,
			context,
			FontType::SignedDistanceField
		)
	{

	}
}