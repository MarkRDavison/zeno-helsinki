#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/StatusCatalog.hpp>
#include <Services/StatusCategoryCatalog.hpp>
#include <Status.hpp>
#include <stdexcept>
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
  { "id": "poison", "kind": "dot", "color": [0, 1, 0], "cap": 2 },
  { "id": "burn", "kind": "dot", "color": [1, 0, 0] },
  { "id": "weakness", "kind": "stat", "color": [1, 0, 1] }
]
)json";

	constexpr auto kStatuses = R"json(
[
  { "id": "slow", "category": "slow", "duration": 3, "magnitude": 0.5, "channel": "speed" },
  { "id": "poison", "category": "poison", "duration": 2.5, "interval": 1, "tickDamage": 1, "damageType": "poison", "score": 1, "cap": 1 },
  { "id": "poison-b", "category": "poison", "duration": 1, "interval": 1, "tickDamage": 1, "damageType": "poison", "score": 0, "cap": 1 },
  { "id": "poison-c", "category": "poison", "duration": 4, "interval": 1, "tickDamage": 1, "damageType": "poison", "score": 2, "cap": 1 },
  { "id": "burn", "category": "burn", "duration": 2.5, "interval": 1, "tickDamage": 1, "damageType": "fire" },
  { "id": "weakness", "category": "weakness", "duration": 3, "magnitude": 0.2, "channel": "weakness" }
]
)json";

	struct Loaded
	{
		tower::DamageTypeCatalog types;
		tower::StatusCategoryCatalog categories;
		tower::StatusCatalog statuses;
	};

	Loaded load()
	{
		Loaded loaded;
		loaded.types.loadFromText(kTypes, "damage-types.json");
		loaded.categories.loadFromText(kCategories, "status-categories.json");
		loaded.statuses.loadFromText(kStatuses, "statuses.json", loaded.categories, loaded.types);
		return loaded;
	}

	tower::ApplyStatusOutcome apply(
		std::vector<tower::StatusInstance>& list,
		int& seq,
		const Loaded& loaded,
		const char* id)
	{
		const auto* def = loaded.statuses.find(id);
		const auto* cat = loaded.categories.find(def->category);
		return tower::applyStatus(list, *def, *cat, loaded.statuses, seq);
	}
}

TEST_CASE("apply under cap appends", "[tower][combat]")
{
	const auto loaded = load();
	std::vector<tower::StatusInstance> list;
	int seq = 0;
	CHECK(apply(list, seq, loaded, "poison") == tower::ApplyStatusOutcome::Applied);
	REQUIRE(list.size() == 1);
	CHECK(list[0].remaining == Catch::Approx(2.5f));
}

TEST_CASE("apply ignores worse than all at cap", "[tower][combat]")
{
	const auto loaded = load();
	std::vector<tower::StatusInstance> list;
	int seq = 0;
	apply(list, seq, loaded, "poison");
	apply(list, seq, loaded, "poison-c");
	CHECK(apply(list, seq, loaded, "poison-b") == tower::ApplyStatusOutcome::Ignored);
	REQUIRE(list.size() == 2);
}

TEST_CASE("apply evicts lowest score", "[tower][combat]")
{
	const auto loaded = load();
	std::vector<tower::StatusInstance> list;
	int seq = 0;
	apply(list, seq, loaded, "poison");
	apply(list, seq, loaded, "poison-b");
	CHECK(apply(list, seq, loaded, "poison-c") == tower::ApplyStatusOutcome::Applied);
	REQUIRE(list.size() == 2);
	CHECK(list[0].defId == "poison");
	CHECK(list[1].defId == "poison-c");
}

TEST_CASE("apply tie prefers longer remaining then newest", "[tower][combat]")
{
	const auto loaded = load();
	std::vector<tower::StatusInstance> list;
	int seq = 0;
	apply(list, seq, loaded, "poison");
	apply(list, seq, loaded, "poison-b");
	list[0].remaining = 0.5f;
	list[1].remaining = 0.5f;
	CHECK(apply(list, seq, loaded, "poison") == tower::ApplyStatusOutcome::Applied);
	REQUIRE(list.size() == 2);
}

TEST_CASE("both caps full evicts in the def intersection", "[tower][combat]")
{
	const auto loaded = load();
	std::vector<tower::StatusInstance> list;
	int seq = 0;
	apply(list, seq, loaded, "poison");
	apply(list, seq, loaded, "poison-b");
	CHECK(apply(list, seq, loaded, "poison") == tower::ApplyStatusOutcome::Applied);
	int poisonCount = 0;
	for (const auto& instance : list)
	{
		if (instance.defId == "poison")
		{
			++poisonCount;
		}
	}

	CHECK(poisonCount == 1);
	CHECK(list.size() == 2);
}

TEST_CASE("status duration expire removes instance", "[tower][combat]")
{
	std::vector<tower::StatusInstance> list;
	list.push_back(tower::StatusInstance{ .defId = "slow", .remaining = 0.5f });
	tower::tickStatusDurations(list, 0.5f);
	CHECK(list.empty());
}
