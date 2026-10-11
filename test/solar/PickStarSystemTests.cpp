#include <Views/GalaxyDrawConstants.hpp>
#include <World/PickStarSystem.hpp>
#include <World/StarterMap.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace PickStarSystemTests
{

TEST_CASE("pick hits a starter-map system at its position", "[slr][PickStarSystem]")
{
	const WorldData world = makeStarterMap();
	const float radius = GalaxyNodeSize * 0.5f;
	const auto* helion = world.systemById(1);
	REQUIRE(helion != nullptr);

	const auto hit = pickStarSystem(world, helion->position, radius);
	REQUIRE(hit.has_value());
	REQUIRE(*hit == 1);
}

TEST_CASE("pick misses far from every system", "[slr][PickStarSystem]")
{
	const WorldData world = makeStarterMap();
	const auto hit = pickStarSystem(world, glm::vec2(-10000.0f, -10000.0f), GalaxyNodeSize * 0.5f);
	REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("pick chooses the nearer system when radii overlap", "[slr][PickStarSystem]")
{
	WorldData world;
	world.systems = {
		StarSystem{ .id = 2, .name = "Near", .position = { 0.0f, 0.0f } },
		StarSystem{ .id = 1, .name = "Far", .position = { 10.0f, 0.0f } },
	};

	const auto hit = pickStarSystem(world, glm::vec2(4.0f, 0.0f), 20.0f);
	REQUIRE(hit.has_value());
	REQUIRE(*hit == 2);
}

TEST_CASE("pick prefers smaller id when distances are equal", "[slr][PickStarSystem]")
{
	WorldData world;
	world.systems = {
		StarSystem{ .id = 8, .name = "Right", .position = { 10.0f, 0.0f } },
		StarSystem{ .id = 3, .name = "Left", .position = { 0.0f, 0.0f } },
	};

	const auto hit = pickStarSystem(world, glm::vec2(5.0f, 0.0f), 20.0f);
	REQUIRE(hit.has_value());
	REQUIRE(*hit == 3);
}

TEST_CASE("pick on an empty world returns nothing", "[slr][PickStarSystem]")
{
	const WorldData world;
	const auto hit = pickStarSystem(world, glm::vec2(0.0f, 0.0f), 100.0f);
	REQUIRE_FALSE(hit.has_value());
}

}
}
