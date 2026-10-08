#include <catch2/catch_test_macros.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <Services/GameStateService.hpp>
#include <Services/LevelCatalog.hpp>
#include <Services/MatchContext.hpp>
#include <TestCatalogJson.hpp>

namespace
{
	constexpr auto kLevel = R"json(
{
  "id": "inject-gold",
  "boardSize": [8, 8],
  "startGold": 100,
  "startLives": 3,
  "killGold": 10,
  "waveClearBonus": 25,
  "buildTimer": 20,
  "paths": [
    { "name": "main", "path": [ [0, 0], [0, 1] ] }
  ],
  "entities": [],
  "waves": [
    {
      "name": "1",
      "streams": [
        {
          "name": "main",
          "path": "main",
          "spawnInterval": 1,
          "spawns": [ { "id": "runner", "count": 1 } ]
        }
      ]
    }
  ]
}
)json";

	struct Fixture
	{
		tower::DamageTypeCatalog types;
		tower::CreepCatalog creeps;
		tower::EntityCatalog entities;
		tower::LevelCatalog level;

		Fixture()
		{
			types.loadFromText(tower::test::kTypes, "damage-types.json");
			creeps.loadFromText(tower::test::kCreeps, "creeps.json", types);
			level.loadFromText(kLevel, "level.json", creeps, entities);
		}
	};
}

TEST_CASE("skirmish GameStateService gold ignores startingGoldRank", "[tower][match][inject]")
{
	Fixture fixture;
	tower::MatchContext match;
	match.startingGoldRank = 2;
	tower::GameStateService state(fixture.level, match);
	CHECK(state.gold() == 100);
}

TEST_CASE("campaign GameStateService gold adds rank bonus", "[tower][match][inject]")
{
	Fixture fixture;
	tower::MatchContext match;
	match.campaign = true;
	match.startingGoldRank = 2;
	tower::GameStateService state(fixture.level, match);
	CHECK(state.gold() == 100 + 2 * tower::kStartingGoldPerRank);
}

TEST_CASE("campaign GameStateService gold with rank 0 is level start", "[tower][match][inject]")
{
	Fixture fixture;
	tower::MatchContext match;
	match.campaign = true;
	tower::GameStateService state(fixture.level, match);
	CHECK(state.gold() == 100);
}
