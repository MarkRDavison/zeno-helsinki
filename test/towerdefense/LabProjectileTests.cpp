#include <catch2/catch_test_macros.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <stdexcept>
#include <Services/ProjectileCatalog.hpp>
#include <Services/StatusCatalog.hpp>
#include <Services/StatusCategoryCatalog.hpp>
#include <string>

namespace
{
	constexpr auto kTypes = R"json(
[
  { "id": "physical", "name": "Physical", "description": "x" },
  { "id": "fire", "name": "Fire", "description": "y" },
  { "id": "poison", "name": "Poison", "description": "z" }
]
)json";

	constexpr auto kCategories = R"json(
[
  { "id": "slow", "kind": "stat", "color": [0, 0, 1] },
  { "id": "poison", "kind": "dot", "color": [0, 1, 0] },
  { "id": "burn", "kind": "dot", "color": [1, 0, 0] },
  { "id": "weakness", "kind": "stat", "color": [1, 0, 1] }
]
)json";

	constexpr auto kStatuses = R"json(
[
  { "id": "slow", "category": "slow", "duration": 3, "magnitude": 0.5, "channel": "speed" },
  { "id": "poison", "category": "poison", "duration": 2.5, "interval": 1, "tickDamage": 1, "damageType": "poison" },
  { "id": "burn", "category": "burn", "duration": 2.5, "interval": 1, "tickDamage": 1, "damageType": "fire" },
  { "id": "weakness", "category": "weakness", "duration": 3, "magnitude": 0.2, "channel": "weakness" }
]
)json";
}

TEST_CASE("match projectiles.json is bolt only", "[tower][combat][catalog]")
{
	tower::DamageTypeCatalog types;
	types.loadFromText(kTypes, "damage-types.json");
	tower::StatusCategoryCatalog categories;
	categories.loadFromText(kCategories, "status-categories.json");
	tower::StatusCatalog statuses;
	statuses.loadFromText(kStatuses, "statuses.json", categories, types);
	tower::ProjectileCatalog projectiles;
	projectiles.loadFromText(
		R"json([
  { "id": "bolt", "model": "marker", "damage": 1, "damageType": "physical", "speed": 6, "hitRadius": 0.35, "y": 0.4, "scale": [0.15, 0.15, 0.15] }
])json",
		"projectiles.json",
		types,
		statuses);
	REQUIRE(projectiles.all().size() == 1);
	REQUIRE(projectiles.find("bolt") != nullptr);
	CHECK(projectiles.find("bolt")->statuses.empty());
	CHECK(projectiles.find("lab_slow_bolt") == nullptr);
}

TEST_CASE("lab projectiles list v1 status ids and bolt stays empty", "[tower][combat][catalog]")
{
	tower::DamageTypeCatalog types;
	types.loadFromText(kTypes, "damage-types.json");
	tower::StatusCategoryCatalog categories;
	categories.loadFromText(kCategories, "status-categories.json");
	tower::StatusCatalog statuses;
	statuses.loadFromText(kStatuses, "statuses.json", categories, types);
	tower::ProjectileCatalog projectiles;
	projectiles.loadFromText(
		R"json([
  { "id": "bolt", "model": "marker", "damage": 1, "damageType": "physical", "speed": 6, "hitRadius": 0.35, "y": 0.4, "scale": [0.15, 0.15, 0.15] },
  { "id": "lab_slow_bolt", "model": "marker", "damage": 1, "damageType": "physical", "speed": 6, "hitRadius": 0.35, "y": 0.4, "scale": [0.15, 0.15, 0.15], "statuses": ["slow"] },
  { "id": "lab_poison_bolt", "model": "marker", "damage": 1, "damageType": "physical", "speed": 6, "hitRadius": 0.35, "y": 0.4, "scale": [0.15, 0.15, 0.15], "statuses": ["poison"] },
  { "id": "lab_burn_bolt", "model": "marker", "damage": 1, "damageType": "physical", "speed": 6, "hitRadius": 0.35, "y": 0.4, "scale": [0.15, 0.15, 0.15], "statuses": ["burn"] },
  { "id": "lab_weakness_bolt", "model": "marker", "damage": 1, "damageType": "physical", "speed": 6, "hitRadius": 0.35, "y": 0.4, "scale": [0.15, 0.15, 0.15], "statuses": ["weakness"] }
])json",
		"projectiles.json",
		types,
		statuses);

	REQUIRE(projectiles.find("bolt") != nullptr);
	CHECK(projectiles.find("bolt")->statuses.empty());
	CHECK(projectiles.find("lab_slow_bolt")->statuses == std::vector<std::string>{ "slow" });
	CHECK(projectiles.find("lab_poison_bolt")->statuses == std::vector<std::string>{ "poison" });
	CHECK(projectiles.find("lab_burn_bolt")->statuses == std::vector<std::string>{ "burn" });
	CHECK(projectiles.find("lab_weakness_bolt")->statuses == std::vector<std::string>{ "weakness" });
}

TEST_CASE("unknown projectile status is load fail", "[tower][combat][catalog]")
{
	tower::DamageTypeCatalog types;
	types.loadFromText(kTypes, "damage-types.json");
	tower::StatusCategoryCatalog categories;
	categories.loadFromText(kCategories, "status-categories.json");
	tower::StatusCatalog statuses;
	statuses.loadFromText(kStatuses, "statuses.json", categories, types);
	tower::ProjectileCatalog projectiles;
	CHECK_THROWS_AS(
		projectiles.loadFromText(
			R"json([
  { "id": "bolt", "model": "marker", "damage": 1, "damageType": "physical", "speed": 6, "hitRadius": 0.35, "y": 0.4, "scale": [0.15, 0.15, 0.15], "statuses": ["nope"] }
])json",
			"projectiles.json",
			types,
			statuses),
		std::runtime_error);
}
