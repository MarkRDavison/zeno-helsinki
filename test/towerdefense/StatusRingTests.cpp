#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/StatusCatalog.hpp>
#include <Services/StatusCategoryCatalog.hpp>
#include <Status.hpp>
#include <string>
#include <vector>

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
  { "id": "slow", "kind": "stat", "color": [0.1, 0.2, 0.3] },
  { "id": "poison", "kind": "dot", "color": [0.25, 0.85, 0.3] },
  { "id": "burn", "kind": "dot", "color": [1.0, 0.0, 0.0] },
  { "id": "weakness", "kind": "stat", "color": [0.9, 0.1, 0.8] }
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

TEST_CASE("status rings one per category oldest innermost", "[tower][combat]")
{
	tower::DamageTypeCatalog types;
	types.loadFromText(kTypes, "damage-types.json");
	tower::StatusCategoryCatalog categories;
	categories.loadFromText(kCategories, "status-categories.json");
	tower::StatusCatalog statuses;
	statuses.loadFromText(kStatuses, "statuses.json", categories, types);

	std::vector<tower::StatusInstance> list{
		tower::StatusInstance{ .defId = "poison", .seq = 0 },
		tower::StatusInstance{ .defId = "slow", .seq = 1 },
		tower::StatusInstance{ .defId = "poison", .seq = 2 }
	};
	const auto rings = tower::statusRings(list, statuses, categories, 0.4f);
	REQUIRE(rings.size() == 2);
	CHECK(rings[0].categoryId == "poison");
	CHECK(rings[1].categoryId == "slow");
	CHECK(rings[0].color.r == Catch::Approx(0.25f));
	CHECK(rings[0].scaleXZ == Catch::Approx(0.4f * tower::kStatusRingBaseScale));
	CHECK(rings[1].scaleXZ
		== Catch::Approx(0.4f * (tower::kStatusRingBaseScale + tower::kStatusRingScaleStep)));
}

TEST_CASE("no statuses means no rings", "[tower][combat]")
{
	tower::DamageTypeCatalog types;
	types.loadFromText(kTypes, "damage-types.json");
	tower::StatusCategoryCatalog categories;
	categories.loadFromText(kCategories, "status-categories.json");
	tower::StatusCatalog statuses;
	statuses.loadFromText(kStatuses, "statuses.json", categories, types);
	CHECK(tower::statusRings({}, statuses, categories, 0.4f).empty());
}
