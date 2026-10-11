#pragma once

#include <helsinki/Renderer/Export.hpp>

#include <string>

namespace hl
{

	HELSINKI_RENDERER_EXPORT std::string rendererShaderRoot();

	struct RendererShaderRootPath
	{
		operator std::string() const
		{
			return rendererShaderRoot();
		}
	};

	inline const RendererShaderRootPath RendererShaderRoot{};

}
