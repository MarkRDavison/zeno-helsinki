#include <World/StarterMap.hpp>
#include <catch2/catch_test_macros.hpp>
#include <set>
#include <unordered_set>

namespace slr
{
namespace StarterMapTests
{

TEST_CASE("starter map has systems and hyperlanes", "[slr][StarterMap]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(world.systems.size() >= 9);
	REQUIRE_FALSE(world.hyperlanes.empty());
}

TEST_CASE("starter map system ids are unique", "[slr][StarterMap]")
{
	const WorldData world = makeStarterMap();
	std::set<long long> ids;
	for (const auto& system : world.systems)
	{
		REQUIRE(system.id != 0);
		REQUIRE(ids.insert(system.id).second);
	}
}

TEST_CASE("starter map lanes reference existing systems and store a less than b", "[slr][StarterMap]")
{
	const WorldData world = makeStarterMap();
	std::unordered_set<long long> ids;
	for (const auto& system : world.systems)
	{
		ids.insert(system.id);
	}

	for (const auto& lane : world.hyperlanes)
	{
		REQUIRE(lane.a < lane.b);
		REQUIRE(ids.contains(lane.a));
		REQUIRE(ids.contains(lane.b));
		REQUIRE(world.systemById(lane.a) != nullptr);
		REQUIRE(world.systemById(lane.b) != nullptr);
	}
}

}
}
