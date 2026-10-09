#include <catch2/catch_test_macros.hpp>
#include <Core/TileCoordinates.hpp>

using drl::pixelToTile;

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
