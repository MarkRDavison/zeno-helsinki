#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/StatusCatalog.hpp>
#include <Services/StatusCategoryCatalog.hpp>
#include <stdexcept>
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
  { "id": "slow", "kind": "stat", "color": [0.35, 0.55, 1.0] },
  { "id": "poison", "kind": "dot", "color": [0.25, 0.85, 0.3] },
  { "id": "burn", "kind": "dot", "color": [1.0, 0.0, 0.0] },
  { "id": "weakness", "kind": "stat", "color": [0.85, 0.35, 0.9] }
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

	tower::DamageTypeCatalog types()
	{
		tower::DamageTypeCatalog catalog;
		catalog.loadFromText(kTypes, "damage-types.json");
		return catalog;
	}

	tower::StatusCategoryCatalog categories()
	{
		tower::StatusCategoryCatalog catalog;
		catalog.loadFromText(kCategories, "status-categories.json");
		return catalog;
	}
}

TEST_CASE("status catalogs parse v1 ids and omit defaults", "[tower][combat][catalog]")
{
	const auto cats = categories();
	REQUIRE(cats.find("burn") != nullptr);
	CHECK(cats.find("burn")->color.r == Catch::Approx(1.0f));
	CHECK(cats.find("slow")->cap == 1);

	tower::StatusCatalog statuses;
	statuses.loadFromText(kStatuses, "statuses.json", cats, types());
	REQUIRE(statuses.find("poison") != nullptr);
	CHECK(statuses.find("poison")->score == 0);
	CHECK(statuses.find("poison")->cap == 1);
	CHECK(statuses.find("poison")->damageType == "poison");
	CHECK(statuses.find("burn")->damageType == "fire");
	CHECK(statuses.find("slow")->channel == "speed");
	CHECK(statuses.find("weakness")->magnitude == Catch::Approx(0.2f));
}

TEST_CASE("status categories reject unknown kind", "[tower][combat][catalog]")
{
	tower::StatusCategoryCatalog catalog;
	CHECK_THROWS_AS(
		catalog.loadFromText(
			R"json([
  { "id": "slow", "kind": "aura", "color": [0, 0, 1] },
  { "id": "poison", "kind": "dot", "color": [0, 1, 0] },
  { "id": "burn", "kind": "dot", "color": [1, 0, 0] },
  { "id": "weakness", "kind": "stat", "color": [1, 0, 1] }
])json",
			"status-categories.json"),
		std::runtime_error);
}

TEST_CASE("status defs reject unknown category", "[tower][combat][catalog]")
{
	tower::StatusCatalog statuses;
	CHECK_THROWS_AS(
		statuses.loadFromText(
			R"json([
  { "id": "slow", "category": "missing", "duration": 3, "magnitude": 0.5, "channel": "speed" },
  { "id": "poison", "category": "poison", "duration": 2.5, "interval": 1, "tickDamage": 1, "damageType": "poison" },
  { "id": "burn", "category": "burn", "duration": 2.5, "interval": 1, "tickDamage": 1, "damageType": "fire" },
  { "id": "weakness", "category": "weakness", "duration": 3, "magnitude": 0.2, "channel": "weakness" }
])json",
			"statuses.json",
			categories(),
			types()),
		std::runtime_error);
}

TEST_CASE("status defs reject unknown channel", "[tower][combat][catalog]")
{
	tower::StatusCatalog statuses;
	CHECK_THROWS_AS(
		statuses.loadFromText(
			R"json([
  { "id": "slow", "category": "slow", "duration": 3, "magnitude": 0.5, "channel": "laser" },
  { "id": "poison", "category": "poison", "duration": 2.5, "interval": 1, "tickDamage": 1, "damageType": "poison" },
  { "id": "burn", "category": "burn", "duration": 2.5, "interval": 1, "tickDamage": 1, "damageType": "fire" },
  { "id": "weakness", "category": "weakness", "duration": 3, "magnitude": 0.2, "channel": "weakness" }
])json",
			"statuses.json",
			categories(),
			types()),
		std::runtime_error);
}

TEST_CASE("status defs reject unknown tick type", "[tower][combat][catalog]")
{
	tower::StatusCatalog statuses;
	CHECK_THROWS_AS(
		statuses.loadFromText(
			R"json([
  { "id": "slow", "category": "slow", "duration": 3, "magnitude": 0.5, "channel": "speed" },
  { "id": "poison", "category": "poison", "duration": 2.5, "interval": 1, "tickDamage": 1, "damageType": "laser" },
  { "id": "burn", "category": "burn", "duration": 2.5, "interval": 1, "tickDamage": 1, "damageType": "fire" },
  { "id": "weakness", "category": "weakness", "duration": 3, "magnitude": 0.2, "channel": "weakness" }
])json",
			"statuses.json",
			categories(),
			types()),
		std::runtime_error);
}
