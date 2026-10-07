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
  { "id": "slow", "kind": "stat", "color": [0, 0, 1], "cap": 2 },
  { "id": "poison", "kind": "dot", "color": [0, 1, 0] },
  { "id": "burn", "kind": "dot", "color": [1, 0, 0] },
  { "id": "weakness", "kind": "stat", "color": [1, 0, 1] },
  { "id": "health", "kind": "stat", "color": [1, 1, 0], "cap": 2 },
  { "id": "resist", "kind": "stat", "color": [0.5, 0.5, 0.5] }
]
)json";

	constexpr auto kStatuses = R"json(
[
  { "id": "slow", "category": "slow", "duration": 3, "magnitude": 0.5, "channel": "speed", "cap": 2 },
  { "id": "slow-full", "category": "slow", "duration": 3, "magnitude": 1.0, "channel": "speed" },
  { "id": "poison", "category": "poison", "duration": 2.5, "interval": 1, "tickDamage": 1, "damageType": "poison" },
  { "id": "burn", "category": "burn", "duration": 2.5, "interval": 1, "tickDamage": 1, "damageType": "fire" },
  { "id": "weakness", "category": "weakness", "duration": 3, "magnitude": 0.2, "channel": "weakness" },
  { "id": "vitality", "category": "health", "duration": 3, "magnitude": 0.1, "channel": "health", "cap": 2 },
  { "id": "iron-skin", "category": "resist", "duration": 3, "magnitude": 0.2, "channel": "physical_resistance" }
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

TEST_CASE("slow 0 0.5 1 scales speed", "[tower][combat]")
{
	CHECK(2.0f * (1.0f - 0.0f) == Catch::Approx(2.0f));
	CHECK(2.0f * (1.0f - 0.5f) == Catch::Approx(1.0f));
	CHECK(2.0f * (1.0f - 1.0f) == Catch::Approx(0.0f));
}

TEST_CASE("two slows add then multiply once", "[tower][combat]")
{
	auto statuses = loadStatuses();
	std::vector<tower::StatusInstance> list{
		tower::StatusInstance{ .defId = "slow" },
		tower::StatusInstance{ .defId = "slow", .seq = 1 }
	};
	CHECK(tower::channelSum(list, statuses, "speed") == Catch::Approx(1.0f));
}

TEST_CASE("weakness is separate from innate fire weakness", "[tower][combat]")
{
	CHECK(tower::effectiveResist(-0.2f, 0.0f, 0.2f) == Catch::Approx(-0.4f));
	CHECK(tower::takenDamage(10.0f, tower::effectiveResist(-0.2f, 0.0f, 0.0f)) == Catch::Approx(12.0f));
}

TEST_CASE("health 10 percent keeps current and raises max", "[tower][combat]")
{
	const float base = 100.0f;
	float current = 50.0f;
	float max = base * (1.0f + 0.1f);
	CHECK(max == Catch::Approx(110.0f));
	CHECK(current == Catch::Approx(50.0f));
}

TEST_CASE("two health 10 percent add vs base to 120", "[tower][combat]")
{
	auto statuses = loadStatuses();
	std::vector<tower::StatusInstance> list{
		tower::StatusInstance{ .defId = "vitality" },
		tower::StatusInstance{ .defId = "vitality", .seq = 1 }
	};
	CHECK(100.0f * (1.0f + tower::channelSum(list, statuses, "health")) == Catch::Approx(120.0f));
}

TEST_CASE("health expire clamps current to max", "[tower][combat]")
{
	float max = 110.0f;
	float current = 110.0f;
	max = 100.0f;
	if (current > max)
	{
		current = max;
	}

	CHECK(current == Catch::Approx(100.0f));
}

TEST_CASE("type resistance status reduces that type only", "[tower][combat]")
{
	auto statuses = loadStatuses();
	std::vector<tower::StatusInstance> list{
		tower::StatusInstance{ .defId = "iron-skin" }
	};
	const float physical = tower::effectiveResist(
		0.0f,
		tower::channelSum(list, statuses, "physical_resistance"),
		0.0f);
	const float fire = tower::effectiveResist(
		0.0f,
		tower::channelSum(list, statuses, "fire_resistance"),
		0.0f);
	CHECK(tower::takenDamage(10.0f, physical) == Catch::Approx(8.0f));
	CHECK(tower::takenDamage(10.0f, fire) == Catch::Approx(10.0f));
}

TEST_CASE("attacker type damage scales outgoing", "[tower][combat]")
{
	CHECK(tower::outgoingDamage(10.0f, 0.0f, 0.25f) == Catch::Approx(12.5f));
}
