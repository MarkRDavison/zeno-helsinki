#include <helsinki/Renderer/RendererShaderRoot.hpp>
#include <helsinki/System/RuntimeRoot.hpp>
#include "RendererShaderRootBaked.hpp"

#include <filesystem>
#include <string>

namespace hl
{

	std::string rendererShaderRoot()
	{
		const auto deployed = std::filesystem::path(executableDirectory()) / "data" / "shaders";
		if (std::filesystem::exists(deployed / "text.vert"))
		{
			return deployed.generic_string();
		}

		return RendererShaderRootBaked;
	}

}
