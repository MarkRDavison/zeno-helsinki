#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Views/ShaftInspect.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{
namespace ShaftInspectTests
{

	TEST_CASE("playerShaftDigCost is 100 times level", "[drl][ShaftInspect]")
	{
		REQUIRE(playerShaftDigCost(0) == 0);
		REQUIRE(playerShaftDigCost(1) == 100);
		REQUIRE(playerShaftDigCost(4) == 400);
	}

	TEST_CASE("nextShaftLevel is shaftLevel plus one", "[drl][ShaftInspect]")
	{
		REQUIRE(nextShaftLevel(-1) == 0);
		REQUIRE(nextShaftLevel(3) == 4);
	}

	TEST_CASE("formatNextShaftTooltip uses the money label", "[drl][ShaftInspect]")
	{
		REQUIRE(formatNextShaftTooltip(400, "Money") == "Next shaft: 400 Money");
	}

	TEST_CASE("pickDrillAt hits the drill tile", "[drl][ShaftInspect]")
	{
		TerrainData terrain{};
		terrain.shaftLevel = 3;
		const glm::vec2 center{ 32.0f, 4.0f * 64.0f + 32.0f };
		REQUIRE(pickDrillAt(terrain, center, 0.0f, 0.0f, 64.0f));
	}

	TEST_CASE("pickDrillAt misses ladder and side tiles", "[drl][ShaftInspect]")
	{
		TerrainData terrain{};
		terrain.shaftLevel = 3;
		const glm::vec2 ladder{ 32.0f, 3.0f * 64.0f + 32.0f };
		const glm::vec2 side{ 64.0f + 32.0f, 4.0f * 64.0f + 32.0f };
		REQUIRE_FALSE(pickDrillAt(terrain, ladder, 0.0f, 0.0f, 64.0f));
		REQUIRE_FALSE(pickDrillAt(terrain, side, 0.0f, 0.0f, 64.0f));
	}

}
}
