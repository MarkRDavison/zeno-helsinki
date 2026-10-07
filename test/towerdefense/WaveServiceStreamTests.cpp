#include <catch2/catch_test_macros.hpp>
#include <Services/LevelCatalog.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <Services/GameStateService.hpp>
#include <Services/MatchContext.hpp>
#include <Services/WaveService.hpp>
#include <string>
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

	constexpr auto kCreeps = R"json(
[
  { "id": "runner", "model": "creep_runner", "health": 2, "speed": 2.2 },
  { "id": "tank", "model": "creep_tank", "health": 8, "speed": 0.85 }
]
)json";

	constexpr auto kTwoStreams = R"json(
{
  "id": "test",
  "boardSize": [8, 8],
  "startGold": 100,
  "startLives": 3,
  "killGold": 10,
  "waveClearBonus": 25,
  "buildTimer": 20,
  "paths": [
    { "name": "main", "path": [ [0, 0], [0, 1] ] },
    { "name": "side", "path": [ [7, 0], [7, 1] ] }
  ],
  "entities": [],
  "waves": [
    {
      "name": "opening",
      "streams": [
        {
          "name": "main-runners",
          "path": "main",
          "spawnInterval": 1,
          "spawns": [ { "id": "runner", "count": 2 } ]
        },
        {
          "name": "side-tanks",
          "path": "side",
          "spawnInterval": 0.4,
          "spawns": [ { "id": "tank", "count": 2 } ]
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
		tower::MatchContext match;
		tower::GameStateService gameState;
		tower::WaveService wave;

		Fixture() :
			gameState(level, match),
			wave(gameState, creeps, level)
		{
			types.loadFromText(kTypes, "damage-types.json");
			creeps.loadFromText(kCreeps, "creeps.json", types);
			level.loadFromText(kTwoStreams, "level.json", creeps, entities);
		}
	};

	struct Spawned
	{
		std::string id;
		std::string path;
	};

	std::vector<Spawned> drain(tower::WaveService& wave)
	{
		std::vector<Spawned> spawned;
		while (wave.takeSpawn())
		{
			spawned.push_back({ wave.nextCreep().id, wave.nextPathName() });
		}

		return spawned;
	}
}

TEST_CASE("parallel streams spawn together on wave start", "[tower][paths][wave]")
{
	Fixture fixture;
	REQUIRE(fixture.wave.tryStart());
	const auto first = drain(fixture.wave);
	REQUIRE(first.size() == 2);
	CHECK(first[0].id == "runner");
	CHECK(first[0].path == "main");
	CHECK(first[1].id == "tank");
	CHECK(first[1].path == "side");
	CHECK(fixture.wave.hudWaveLine() == "Wave opening (#1)");
}

TEST_CASE("faster stream spawns again before the slower one", "[tower][paths][wave]")
{
	Fixture fixture;
	REQUIRE(fixture.wave.tryStart());
	drain(fixture.wave);
	fixture.wave.tick(0.4f);
	const auto second = drain(fixture.wave);
	REQUIRE(second.size() == 1);
	CHECK(second[0].id == "tank");
	CHECK(second[0].path == "side");
}

TEST_CASE("tryClear waits until every stream is empty", "[tower][paths][wave]")
{
	Fixture fixture;
	REQUIRE(fixture.wave.tryStart());
	drain(fixture.wave);
	CHECK_FALSE(fixture.wave.tryClear(true));
	fixture.wave.tick(1.0f);
	REQUIRE(drain(fixture.wave).size() == 2);
	CHECK_FALSE(fixture.wave.tryClear(false));
	CHECK(fixture.wave.tryClear(true));
}
