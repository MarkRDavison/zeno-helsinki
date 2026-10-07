#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <PathBlock.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <vector>

namespace
{
	constexpr auto kTypes = R"json(
[
  { "id": "physical", "name": "Physical", "description": "Bolts." },
  { "id": "fire", "name": "Fire", "description": "Fire." },
  { "id": "poison", "name": "Poison", "description": "Poison." }
]
)json";
}

TEST_CASE("range omit is 0", "[tower][creep][range]")
{
	tower::DamageTypeCatalog types;
	types.loadFromText(kTypes, "damage-types.json");
	tower::CreepCatalog creeps;
	creeps.loadFromText(
		R"json([
  { "id": "runner", "model": "creep_runner", "health": 2, "speed": 2.2 },
  { "id": "tank", "model": "creep_tank", "health": 8, "speed": 0.85, "range": 0 }
])json",
		"creeps.json",
		types);
	CHECK(creeps.find("runner")->range == 0.0f);
	CHECK(creeps.find("tank")->range == 0.0f);
}

TEST_CASE("stall at range 0 on the tile before the occupied tile", "[tower][paths][stall]")
{
	const std::vector<tower::TileCoord> path{ { 0, 0 }, { 0, 1 }, { 0, 2 } };
	const tower::PathBlockFootprint gate{ 0, 1, 1, 1 };
	const auto blocked = tower::firstBlockedTile(path, 0, gate);
	REQUIRE(blocked.has_value());
	CHECK(blocked->z == 1);
	const auto approach = tower::stallApproachIndex(path, 0, gate);
	REQUIRE(approach.has_value());
	CHECK(*approach == 0);
	CHECK(path[static_cast<std::size_t>(*approach)].z == 0);

	const glm::vec3 onOccupied{ 0.0f, 0.0f, 1.0f };
	const glm::vec3 occupiedCenter{ 0.0f, 0.0f, 1.0f };
	CHECK(tower::shouldStall(onOccupied, occupiedCenter, 0.0f));

	const glm::vec3 onApproach{ 0.0f, 0.0f, 0.0f };
	CHECK_FALSE(tower::shouldStall(onApproach, occupiedCenter, 0.0f));
}

TEST_CASE("no block after the creep has passed the footprint", "[tower][paths][stall]")
{
	const std::vector<tower::TileCoord> path{ { 0, 0 }, { 0, 1 }, { 0, 2 } };
	const tower::PathBlockFootprint gate{ 0, 1, 1, 1 };
	CHECK_FALSE(tower::firstBlockedTile(path, 2, gate).has_value());
}

TEST_CASE("resume when path entity is gone", "[tower][paths][stall]")
{
	const std::vector<tower::TileCoord> path{ { 0, 0 }, { 0, 1 }, { 0, 2 } };
	CHECK_FALSE(tower::firstBlockedTile(path, 0, tower::PathBlockFootprint{ 9, 9, 1, 1 }).has_value());
}
