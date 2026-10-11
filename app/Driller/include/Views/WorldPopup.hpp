#pragma once

#include <helsinki/System/glm.hpp>
#include <algorithm>
#include <string>

namespace drl
{

	inline constexpr float kWorldPopupDuration = 1.0f;
	inline constexpr float kWorldPopupRisePixels = 32.0f;
	inline constexpr unsigned kWorldPopupFontSize = 20;

	struct WorldPopup
	{
		glm::ivec2 tile{ 0, 0 };
		std::string text;
		float age{ 0.0f };
		int entityId{ -1 };
	};

	inline glm::vec3 worldPopupPosition(
		glm::ivec2 tile,
		float originX,
		float originY,
		float tileSize,
		float age,
		float zoom)
	{
		const float safeZoom = std::max(zoom, 0.001f);
		const float t = std::clamp(age / kWorldPopupDuration, 0.0f, 1.0f);
		const float riseWorld = (t * kWorldPopupRisePixels) / safeZoom;
		return glm::vec3(
			originX + (static_cast<float>(tile.x) + 0.5f) * tileSize,
			originY + (static_cast<float>(tile.y) + 0.5f) * tileSize - riseWorld,
			0.0f);
	}

	inline glm::vec3 worldPopupScale(float zoom)
	{
		const float s = 1.0f / std::max(zoom, 0.001f);
		return glm::vec3(s, s, 1.0f);
	}

	inline glm::vec4 worldPopupColour(float age)
	{
		const float t = std::clamp(age / kWorldPopupDuration, 0.0f, 1.0f);
		const glm::vec3 errorMuted{ 0.86f, 0.58f, 0.58f };
		return glm::vec4(errorMuted, 1.0f - t);
	}

}
