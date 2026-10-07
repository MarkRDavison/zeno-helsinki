#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/ProjectileCatalog.hpp>
#include <Services/StatusCatalog.hpp>
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

	constexpr auto kBolt = R"json(
[
  { "id": "bolt", "model": "marker", "damage": 1, "damageType": "physical", "speed": 6, "hitRadius": 0.35, "y": 0.4, "scale": [0.15, 0.15, 0.15] }
]
)json";

	tower::DamageTypeCatalog loadedTypes()
	{
		tower::DamageTypeCatalog catalog;
		catalog.loadFromText(kTypes, "damage-types.json");
		return catalog;
	}
}

TEST_CASE("bolt is physical only", "[tower][combat][catalog]")
{
	const auto types = loadedTypes();
	tower::ProjectileCatalog projectiles;
	tower::StatusCatalog statuses;
	projectiles.loadFromText(kBolt, "projectiles.json", types, statuses);
	const auto* bolt = projectiles.find("bolt");
	REQUIRE(bolt != nullptr);
	CHECK(bolt->damageType == "physical");
	CHECK(bolt->damage == Catch::Approx(1.0f));
	CHECK(bolt->statuses.empty());
	CHECK(projectiles.all().size() == 1);
}

TEST_CASE("unknown projectile damageType is load fail", "[tower][combat][catalog]")
{
	const auto types = loadedTypes();
	tower::ProjectileCatalog projectiles;
	CHECK_THROWS_AS(
		projectiles.loadFromText(
			R"json([
  { "id": "bolt", "model": "marker", "damage": 1, "damageType": "laser", "speed": 6, "hitRadius": 0.35, "y": 0.4, "scale": [0.15, 0.15, 0.15] }
])json",
			"projectiles.json",
			types,
			tower::StatusCatalog()),
		std::runtime_error);
}

TEST_CASE("missing projectile damageType is load fail", "[tower][combat][catalog]")
{
	const auto types = loadedTypes();
	tower::ProjectileCatalog projectiles;
	CHECK_THROWS_AS(
		projectiles.loadFromText(
			R"json([
  { "id": "bolt", "model": "marker", "damage": 1, "speed": 6, "hitRadius": 0.35, "y": 0.4, "scale": [0.15, 0.15, 0.15] }
])json",
			"projectiles.json",
			types,
			tower::StatusCatalog()),
		std::runtime_error);
}

TEST_CASE("test-only fire projectile loads", "[tower][combat][catalog]")
{
	const auto types = loadedTypes();
	REQUIRE(types.contains("fire"));
	tower::ProjectileCatalog projectiles;
	tower::StatusCatalog statuses;
	projectiles.loadFromText(
		R"json([
  { "id": "ember", "model": "marker", "damage": 1, "damageType": "fire", "speed": 6, "hitRadius": 0.35, "y": 0.4, "scale": [0.15, 0.15, 0.15] }
])json",
		"projectiles.json",
		types,
		statuses);
	const auto* ember = projectiles.find("ember");
	REQUIRE(ember != nullptr);
	CHECK(ember->damageType == "fire");
}
