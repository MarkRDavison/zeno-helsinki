#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{
	constexpr auto kTypes = R"json(
[
  { "id": "physical", "name": "Physical", "description": "Bolts and kinetic hits." },
  { "id": "fire", "name": "Fire", "description": "Explosions and burn hits." },
  { "id": "poison", "name": "Poison", "description": "Poison status ticks." }
]
)json";

	tower::DamageTypeCatalog loadedTypes()
	{
		tower::DamageTypeCatalog catalog;
		catalog.loadFromText(kTypes, "damage-types.json");
		return catalog;
	}

	constexpr auto kCreeps = R"json(
[
  { "id": "runner", "model": "creep_runner", "health": 2, "speed": 2.2 },
  { "id": "tank", "model": "creep_tank", "health": 8, "speed": 0.85, "resist": { "physical": 0.25 } }
]
)json";
}

TEST_CASE("tank physical resist is 0.25", "[tower][combat][catalog]")
{
	const auto types = loadedTypes();
	tower::CreepCatalog creeps;
	creeps.loadFromText(kCreeps, "creeps.json", types);
	const auto* tank = creeps.find("tank");
	REQUIRE(tank != nullptr);
	REQUIRE(tank->resist.contains("physical"));
	CHECK(tank->resist.at("physical") == Catch::Approx(0.25f));
}

TEST_CASE("omitted resist is missing keys at 0", "[tower][combat][catalog]")
{
	const auto types = loadedTypes();
	tower::CreepCatalog creeps;
	creeps.loadFromText(kCreeps, "creeps.json", types);
	const auto* runner = creeps.find("runner");
	REQUIRE(runner != nullptr);
	CHECK(runner->resist.empty());
}

TEST_CASE("unknown resist key is load fail", "[tower][combat][catalog]")
{
	const auto types = loadedTypes();
	tower::CreepCatalog creeps;
	CHECK_THROWS_AS(
		creeps.loadFromText(
			R"json([
  { "id": "runner", "model": "creep_runner", "health": 2, "speed": 2.2 },
  { "id": "tank", "model": "creep_tank", "health": 8, "speed": 0.85, "resist": { "laser": 0.1 } }
])json",
			"creeps.json",
			types),
		std::runtime_error);
}

TEST_CASE("resist above 1 loads clamped and warns", "[tower][combat][catalog]")
{
	const auto types = loadedTypes();
	tower::CreepCatalog creeps;
	std::ostringstream captured;
	auto* previous = std::clog.rdbuf(captured.rdbuf());
	creeps.loadFromText(
		R"json([
  { "id": "runner", "model": "creep_runner", "health": 2, "speed": 2.2 },
  { "id": "tank", "model": "creep_tank", "health": 8, "speed": 0.85, "resist": { "physical": 1.5 } }
])json",
		"creeps.json",
		types);
	std::clog.rdbuf(previous);

	const auto* tank = creeps.find("tank");
	REQUIRE(tank != nullptr);
	CHECK(tank->resist.at("physical") == Catch::Approx(1.0f));
	CHECK(captured.str().find("clamped to 1.0") != std::string::npos);
}

TEST_CASE("test-only fire resist loads", "[tower][combat][catalog]")
{
	const auto types = loadedTypes();
	tower::CreepCatalog creeps;
	creeps.loadFromText(
		R"json([
  { "id": "runner", "model": "creep_runner", "health": 2, "speed": 2.2 },
  { "id": "tank", "model": "creep_tank", "health": 8, "speed": 0.85, "resist": { "fire": -0.2 } }
])json",
		"creeps.json",
		types);
	const auto* tank = creeps.find("tank");
	REQUIRE(tank != nullptr);
	CHECK(tank->resist.at("fire") == Catch::Approx(-0.2f));
}
