#include <Views/ShaftInspect.hpp>
#include <format>

namespace drl
{

	ShaftSpriteBox drillSpriteBox(
		const TerrainData& terrain,
		float originX,
		float originY,
		float tileSize)
	{
		const int level = nextShaftLevel(terrain.shaftLevel);
		return ShaftSpriteBox{
			.left = originX,
			.top = originY + static_cast<float>(level) * tileSize,
			.width = tileSize,
			.height = tileSize
		};
	}

	bool pickDrillAt(
		const TerrainData& terrain,
		glm::vec2 worldPos,
		float originX,
		float originY,
		float tileSize)
	{
		const ShaftSpriteBox box = drillSpriteBox(terrain, originX, originY, tileSize);
		return worldPos.x >= box.left && worldPos.x < box.left + box.width
			&& worldPos.y >= box.top && worldPos.y < box.top + box.height;
	}

	std::string formatNextShaftTooltip(long long cost, std::string_view moneyLabel)
	{
		return std::format("Next shaft: {} {}", cost, moneyLabel);
	}

}
