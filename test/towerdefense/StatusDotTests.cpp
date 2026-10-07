#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Combat.hpp>
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
  { "id": "slow", "kind": "stat", "color": [0, 0, 1] },
  { "id": "poison", "kind": "dot", "color": [0, 1, 0], "cap": 2 },
  { "id": "burn", "kind": "dot", "color": [1, 0, 0] },
  { "id": "weakness", "kind": "stat", "color": [1, 0, 1] }
]
)json";

	constexpr auto kStatuses = R"json(
[
  { "id": "slow", "category": "slow", "duration": 3, "magnitude": 0.5, "channel": "speed" },
  { "id": "poison", "category": "poison", "duration": 2.5, "interval": 1, "tickDamage": 1, "damageType": "poison", "cap": 2 },
  { "id": "poison-fast", "category": "poison", "duration": 2, "interval": 0.5, "tickDamage": 1, "damageType": "poison" },
  { "id": "burn", "category": "burn", "duration": 2.5, "interval": 1, "tickDamage": 1, "damageType": "fire" },
  { "id": "weakness", "category": "weakness", "duration": 3, "magnitude": 0.2, "channel": "weakness" }
]
)json";

	tower::StatusCatalog loadStatuses()
	{
		tower::DamageTypeCatalog types;
		types.loadFromText(kTypes, "damage-types.json");
		tower::StatusCategoryCatalog categories;
		categories.loadFromText(kCategories, "status-categories.json");
		tower::StatusCatalog statuses;
		statuses.loadFromText(kStatuses, "statuses.json", categories, types);
		return statuses;
	}
}

TEST_CASE("dot 2.5s interval 1 ticks at 1 and 2 only", "[tower][combat]")
{
	auto statuses = loadStatuses();
	std::vector<tower::StatusInstance> list{
		tower::StatusInstance{ .defId = "poison", .remaining = 2.5f }
	};
	auto ticks = tower::tickDots(list, statuses, 1.0f);
	REQUIRE(ticks.size() == 1);
	ticks = tower::tickDots(list, statuses, 1.0f);
	REQUIRE(ticks.size() == 1);
	ticks = tower::tickDots(list, statuses, 0.5f);
	CHECK(ticks.empty());
	CHECK(list.empty());
}

TEST_CASE("dot interval 0.5 over 1s ticks twice", "[tower][combat]")
{
	auto statuses = loadStatuses();
	std::vector<tower::StatusInstance> list{
		tower::StatusInstance{ .defId = "poison-fast", .remaining = 2.0f }
	};
	const auto ticks = tower::tickDots(list, statuses, 1.0f);
	CHECK(ticks.size() == 2);
}

TEST_CASE("poison ticks ignore physical resist", "[tower][combat]")
{
	CHECK(tower::takenDamage(1.0f, 0.25f) == Catch::Approx(0.75f));
	CHECK(tower::takenDamage(1.0f, tower::resistOf({ { "physical", 0.25f } }, "poison"))
		== Catch::Approx(1.0f));
}

TEST_CASE("burn ticks use fire type", "[tower][combat]")
{
	auto statuses = loadStatuses();
	std::vector<tower::StatusInstance> list{
		tower::StatusInstance{ .defId = "burn", .remaining = 2.5f }
	};
	const auto ticks = tower::tickDots(list, statuses, 1.0f);
	REQUIRE(ticks.size() == 1);
	CHECK(ticks[0].damageType == "fire");
	CHECK(tower::takenDamage(ticks[0].damage, -0.2f) == Catch::Approx(1.2f));
}

TEST_CASE("two poison stacks tick twice", "[tower][combat]")
{
	auto statuses = loadStatuses();
	std::vector<tower::StatusInstance> list{
		tower::StatusInstance{ .defId = "poison", .remaining = 2.5f },
		tower::StatusInstance{ .defId = "poison", .remaining = 2.5f, .seq = 1 }
	};
	const auto ticks = tower::tickDots(list, statuses, 1.0f);
	CHECK(ticks.size() == 2);
}

TEST_CASE("dot overkill kills", "[tower][combat]")
{
	float current = 0.5f;
	tower::applyHit(current, 1.0f, 0.0f);
	CHECK(tower::isDead(current));
}
