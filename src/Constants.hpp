#pragma once

#include <string>

namespace Constants
{
	const std::string Title = "zeno helsinki";

    inline std::string GetAssetPath(std::string assetName)
    {
        return ASSETS_PATH "" + assetName;
    }
}