#pragma once

#include <Entities/Data/TerrainData.hpp>
#include <Services/GameCommandService.hpp>
#include <helsinki/System/glm.hpp>
#include <optional>
#include <string>
#include <string_view>

namespace drl
{

	struct ShaftSpriteBox
	{
		float left{ 0.0f };
		float top{ 0.0f };
		float width{ 0.0f };
		float height{ 0.0f };
	};

	struct DrillHoverTooltip
	{
		glm::vec2 screenPos{ 0.0f, 0.0f };
		glm::vec2 screenSize{ 0.0f, 0.0f };
		std::string text;
	};

	inline int nextShaftLevel(int shaftLevel)
	{
		return shaftLevel + 1;
	}

	inline long long playerShaftDigCost(int level)
	{
		return kPlayerShaftDigCostPerLevel * static_cast<long long>(level);
	}

	ShaftSpriteBox drillSpriteBox(
		const TerrainData& terrain,
		float originX,
		float originY,
		float tileSize);

	bool pickDrillAt(
		const TerrainData& terrain,
		glm::vec2 worldPos,
		float originX,
		float originY,
		float tileSize);

	std::string formatNextShaftTooltip(long long cost, std::string_view moneyLabel);

}
