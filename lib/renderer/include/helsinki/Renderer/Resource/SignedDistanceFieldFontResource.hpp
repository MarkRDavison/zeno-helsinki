#pragma once

#include <helsinki/Renderer/Resource/FontResource.hpp>

namespace hl
{

	class SignedDistanceFieldFontResource : public FontResource
	{
	public:
		explicit SignedDistanceFieldFontResource(
			const std::string& id,
			ResourceContext& context);

	private:

	};

}