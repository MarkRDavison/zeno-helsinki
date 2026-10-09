#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <Core/TileCoordinates.hpp>

namespace drl
{
namespace TileCoordinatesTests
{

TEST_CASE("pixelToTile maps origin to shaft cell", "[drl][TileCoordinates]")
{
	const glm::ivec2 tile = pixelToTile({ 640.0f, 64.0f }, 640.0f, 64.0f, 64.0f);
	REQUIRE(tile.x == 0);
	REQUIRE(tile.y == 0);
}

TEST_CASE("pixelToTile stays on the same cell until the next tile edge", "[drl][TileCoordinates]")
{
	const glm::ivec2 same = pixelToTile({ 640.0f + 63.9f, 64.0f }, 640.0f, 64.0f, 64.0f);
	REQUIRE(same.x == 0);
	REQUIRE(same.y == 0);

	const glm::ivec2 next = pixelToTile({ 640.0f + 64.0f, 64.0f }, 640.0f, 64.0f, 64.0f);
	REQUIRE(next.x == 1);
	REQUIRE(next.y == 0);
}

TEST_CASE("pixelToTile maps left and right columns", "[drl][TileCoordinates]")
{
	const glm::ivec2 right = pixelToTile({ 640.0f + 64.0f, 64.0f + 64.0f }, 640.0f, 64.0f, 64.0f);
	REQUIRE(right.x == 1);
	REQUIRE(right.y == 1);

	const glm::ivec2 left = pixelToTile({ 640.0f - 1.0f, 64.0f }, 640.0f, 64.0f, 64.0f);
	REQUIRE(left.x == -1);
	REQUIRE(left.y == 0);
}

TEST_CASE("pixelToTile maps above the mine to a negative level", "[drl][TileCoordinates]")
{
	const glm::ivec2 tile = pixelToTile({ 640.0f, 63.0f }, 640.0f, 64.0f, 64.0f);
	REQUIRE(tile.x == 0);
	REQUIRE(tile.y == -1);
}

TEST_CASE("atlasUvRect insets half a texel and stays inside the sheet", "[drl][TileCoordinates]")
{
	constexpr float kTileSize = 64.0f;
	constexpr float kTexSize = 1024.0f;
	const glm::vec4 first = atlasUvRect(0, 0, kTileSize, kTexSize);
	REQUIRE(first.x == Catch::Approx(0.5f / kTexSize));
	REQUIRE(first.y == Catch::Approx(0.5f / kTexSize));
	REQUIRE(first.z == Catch::Approx((kTileSize - 0.5f) / kTexSize));
	REQUIRE(first.w == Catch::Approx((kTileSize - 0.5f) / kTexSize));
	REQUIRE(first.x >= 0.0f);
	REQUIRE(first.y >= 0.0f);

	const glm::vec4 last = atlasUvRect(15, 15, kTileSize, kTexSize);
	REQUIRE(last.z == Catch::Approx((kTexSize - 0.5f) / kTexSize));
	REQUIRE(last.w == Catch::Approx((kTexSize - 0.5f) / kTexSize));
	REQUIRE(last.z <= 1.0f);
	REQUIRE(last.w <= 1.0f);

	const glm::vec4 neighbour = atlasUvRect(1, 0, kTileSize, kTexSize);
	REQUIRE(first.z < neighbour.x);
}

}
}
