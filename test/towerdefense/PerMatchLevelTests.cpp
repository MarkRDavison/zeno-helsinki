#include <catch2/catch_test_macros.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <Services/GameStateService.hpp>
#include <Services/LevelCatalog.hpp>
#include <Services/MatchContext.hpp>
#include <Services/WaveService.hpp>

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

	constexpr auto kMapA = R"json(
{
  "id": "map-a",
  "boardSize": [8, 8],
  "startGold": 50,
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
      "name": "alpha",
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

	constexpr auto kMapB = R"json(
{
  "id": "map-b",
  "boardSize": [8, 8],
  "startGold": 200,
  "startLives": 5,
  "killGold": 4,
  "waveClearBonus": 40,
  "buildTimer": 10,
  "paths": [
    { "name": "main", "path": [ [0, 0], [0, 1] ] }
  ],
  "entities": [],
  "waves": [
    {
      "name": "beta",
      "streams": [
        {
          "name": "main",
          "path": "main",
          "spawnInterval": 1,
          "spawns": [ { "id": "tank", "count": 1 } ]
        }
      ]
    },
    {
      "name": "gamma",
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

	struct Catalogs
	{
		tower::DamageTypeCatalog types;
		tower::CreepCatalog creeps;
		tower::EntityCatalog entities;

		Catalogs()
		{
			types.loadFromText(kTypes, "damage-types.json");
			creeps.loadFromText(kCreeps, "creeps.json", types);
		}
	};

	void load(tower::LevelCatalog& level, Catalogs& catalogs, const char* json, const char* file)
	{
		level.loadFromText(json, file, catalogs.creeps, catalogs.entities);
	}
}

TEST_CASE("two boards keep gold and waves isolated", "[tower][level][match]")
{
	Catalogs catalogs;
	tower::LevelCatalog boardA;
	tower::LevelCatalog boardB;
	load(boardA, catalogs, kMapA, "map-a.json");
	load(boardB, catalogs, kMapB, "map-b.json");

	tower::MatchContext match;
	tower::GameStateService stateA(boardA, match);
	tower::WaveService wavesA(stateA, catalogs.creeps, boardA);
	tower::GameStateService stateB(boardB, match);
	tower::WaveService wavesB(stateB, catalogs.creeps, boardB);

	CHECK(stateA.gold() == 50);
	CHECK(stateB.gold() == 200);
	CHECK(wavesA.hudWaveLine() == "Wave alpha (#1)");
	CHECK(wavesB.hudWaveLine() == "Wave beta (#1)");
	CHECK(boardA.waveCount() == 1);
	CHECK(boardB.waveCount() == 2);
	CHECK(boardA.waveClearBonus() == 25);
	CHECK(boardB.waveClearBonus() == 40);
}

TEST_CASE("reloading the unused loader does not change a snapshotted match", "[tower][level][match]")
{
	Catalogs catalogs;
	tower::LevelCatalog live;
	load(live, catalogs, kMapA, "map-a.json");
	tower::LevelCatalog matchBoard = live;

	tower::MatchContext match;
	tower::GameStateService state(matchBoard, match);
	tower::WaveService waves(state, catalogs.creeps, matchBoard);

	load(live, catalogs, kMapB, "map-b.json");

	CHECK(state.gold() == 50);
	CHECK(waves.hudWaveLine() == "Wave alpha (#1)");
	CHECK(matchBoard.waveCount() == 1);
	CHECK(matchBoard.startGold() == 50);
	CHECK(live.waveCount() == 2);
	CHECK(live.startGold() == 200);
}
