#include <catch2/catch_test_macros.hpp>
#include <Services/LevelCatalog.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <stdexcept>
#include <string>

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

	constexpr auto kHeader = R"json(
  "id": "test",
  "boardSize": [8, 8],
  "startGold": 100,
  "startLives": 3,
  "killGold": 10,
  "waveClearBonus": 25,
  "buildTimer": 20,
)json";

	tower::DamageTypeCatalog types()
	{
		tower::DamageTypeCatalog catalog;
		catalog.loadFromText(kTypes, "damage-types.json");
		return catalog;
	}

	tower::CreepCatalog creeps()
	{
		const auto loadedTypes = types();
		tower::CreepCatalog catalog;
		catalog.loadFromText(kCreeps, "creeps.json", loadedTypes);
		return catalog;
	}

	constexpr auto kEntities = R"json(
[
  { "id": "tree", "model": "detail_tree", "size": [1, 1] },
  { "id": "rocks", "model": "detail_rocks", "size": [1, 1] }
]
)json";

	tower::EntityCatalog entities()
	{
		const auto loadedTypes = types();
		tower::EntityCatalog catalog;
		catalog.loadFromText(kEntities, "entities.json", loadedTypes);
		return catalog;
	}

	void loadLevel(tower::LevelCatalog& level, const std::string& json)
	{
		const auto loadedCreeps = creeps();
		const auto loadedEntities = entities();
		level.loadFromText(json, "level.json", loadedCreeps, loadedEntities);
	}

	std::string wrap(const std::string& body)
	{
		return std::string("{") + kHeader + body + "}";
	}
}

TEST_CASE("migrated single path and stream loads", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	loadLevel(
		level,
		wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1], [0, 2] ] } ],
  "entities": [],
  "waves": [
    {
      "name": "1",
      "streams": [
        {
          "name": "main",
          "path": "main",
          "spawnInterval": 1,
          "spawns": [ { "id": "runner", "count": 5 } ]
        }
      ]
    }
  ]
)json"));
	REQUIRE(level.paths().size() == 1);
	REQUIRE(level.paths()[0].name == "main");
	REQUIRE(level.path("main").size() == 3);
	REQUIRE(level.waves().size() == 1);
	REQUIRE(level.waves()[0].name == "1");
	REQUIRE(level.waves()[0].streams.size() == 1);
	CHECK(level.waves()[0].streams[0].spawnInterval == 1.0f);
	CHECK(level.isPathTile(0, 1));
	CHECK_FALSE(level.isPathTile(1, 0));
}

TEST_CASE("two paths and two streams load", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	loadLevel(
		level,
		wrap(R"json(
  "paths": [
    { "name": "main", "path": [ [0, 0], [0, 1], [0, 2] ] },
    { "name": "side", "path": [ [7, 0], [7, 1], [7, 2] ] }
  ],
  "entities": [],
  "waves": [
    {
      "name": "opening",
      "streams": [
        {
          "name": "main-runners",
          "path": "main",
          "spawnInterval": 0.8,
          "spawns": [ { "id": "runner", "count": 5 } ]
        },
        {
          "name": "side-tanks",
          "path": "side",
          "spawnInterval": 1.2,
          "spawns": [ { "id": "tank", "count": 2 } ]
        }
      ]
    }
  ]
)json"));
	REQUIRE(level.paths().size() == 2);
	CHECK(level.path("side").front().x == 7);
	REQUIRE(level.waves()[0].streams.size() == 2);
	CHECK(level.waves()[0].streams[1].pathName == "side");
}

TEST_CASE("two streams may share a path", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	loadLevel(
		level,
		wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1] ] } ],
  "entities": [],
  "waves": [
    {
      "name": "1",
      "streams": [
        {
          "name": "fast",
          "path": "main",
          "spawnInterval": 0.4,
          "spawns": [ { "id": "runner", "count": 2 } ]
        },
        {
          "name": "slow",
          "path": "main",
          "spawnInterval": 1,
          "spawns": [ { "id": "tank", "count": 1 } ]
        }
      ]
    }
  ]
)json"));
	REQUIRE(level.waves()[0].streams.size() == 2);
	CHECK(level.waves()[0].streams[0].pathName == "main");
	CHECK(level.waves()[0].streams[1].pathName == "main");
}

TEST_CASE("shared path tiles are unbuildable", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	loadLevel(
		level,
		wrap(R"json(
  "paths": [
    { "name": "a", "path": [ [0, 0], [1, 0], [2, 0] ] },
    { "name": "b", "path": [ [1, 0], [1, 1], [1, 2] ] }
  ],
  "entities": [],
  "waves": [
    {
      "name": "1",
      "streams": [
        {
          "name": "a",
          "path": "a",
          "spawnInterval": 1,
          "spawns": [ { "id": "runner", "count": 1 } ]
        }
      ]
    }
  ]
)json"));
	CHECK(level.isPathTile(1, 0));
	CHECK(level.isPathTile(2, 0));
	CHECK(level.isPathTile(1, 2));
}

TEST_CASE("singular path key is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			R"json({
  "id": "test",
  "boardSize": [8, 8],
  "startGold": 100,
  "startLives": 3,
  "killGold": 10,
  "waveClearBonus": 25,
  "buildTimer": 20,
  "path": [ [0, 0], [0, 1] ],
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1] ] } ],
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
})json"),
		std::runtime_error);
}

TEST_CASE("level-wide spawnInterval is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "spawnInterval": 1,
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1] ] } ],
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
)json")),
		std::runtime_error);
}

TEST_CASE("old wave shape is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1] ] } ],
  "entities": [],
  "waves": [ { "id": "1", "spawns": [ { "id": "runner", "count": 1 } ] } ]
)json")),
		std::runtime_error);
}

TEST_CASE("unknown stream path is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1] ] } ],
  "entities": [],
  "waves": [
    {
      "name": "1",
      "streams": [
        {
          "name": "main",
          "path": "side",
          "spawnInterval": 1,
          "spawns": [ { "id": "runner", "count": 1 } ]
        }
      ]
    }
  ]
)json")),
		std::runtime_error);
}

TEST_CASE("duplicate path name is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [
    { "name": "main", "path": [ [0, 0], [0, 1] ] },
    { "name": "main", "path": [ [1, 0], [1, 1] ] }
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
)json")),
		std::runtime_error);
}

TEST_CASE("duplicate stream name in a wave is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1] ] } ],
  "entities": [],
  "waves": [
    {
      "name": "1",
      "streams": [
        {
          "name": "dup",
          "path": "main",
          "spawnInterval": 1,
          "spawns": [ { "id": "runner", "count": 1 } ]
        },
        {
          "name": "dup",
          "path": "main",
          "spawnInterval": 1,
          "spawns": [ { "id": "tank", "count": 1 } ]
        }
      ]
    }
  ]
)json")),
		std::runtime_error);
}

TEST_CASE("empty paths array is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [],
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
)json")),
		std::runtime_error);
}

TEST_CASE("non-adjacent path step is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 2] ] } ],
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
)json")),
		std::runtime_error);
}

TEST_CASE("stream missing spawnInterval is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1] ] } ],
  "entities": [],
  "waves": [
    {
      "name": "1",
      "streams": [
        {
          "name": "main",
          "path": "main",
          "spawns": [ { "id": "runner", "count": 1 } ]
        }
      ]
    }
  ]
)json")),
		std::runtime_error);
}

TEST_CASE("path entry missing name is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "path": [ [0, 0], [0, 1] ] } ],
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
)json")),
		std::runtime_error);
}

TEST_CASE("path entry missing path is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "name": "main" } ],
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
)json")),
		std::runtime_error);
}

TEST_CASE("path with fewer than 2 tiles is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0] ] } ],
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
)json")),
		std::runtime_error);
}

TEST_CASE("off-board path tile is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [8, 0] ] } ],
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
)json")),
		std::runtime_error);
}

TEST_CASE("duplicate wave name is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1] ] } ],
  "entities": [],
  "waves": [
    {
      "name": "1",
      "streams": [
        {
          "name": "a",
          "path": "main",
          "spawnInterval": 1,
          "spawns": [ { "id": "runner", "count": 1 } ]
        }
      ]
    },
    {
      "name": "1",
      "streams": [
        {
          "name": "b",
          "path": "main",
          "spawnInterval": 1,
          "spawns": [ { "id": "runner", "count": 1 } ]
        }
      ]
    }
  ]
)json")),
		std::runtime_error);
}

TEST_CASE("wave missing name is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1] ] } ],
  "entities": [],
  "waves": [
    {
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
)json")),
		std::runtime_error);
}

TEST_CASE("wave missing streams is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1] ] } ],
  "entities": [],
  "waves": [ { "name": "1" } ]
)json")),
		std::runtime_error);
}

TEST_CASE("empty streams array is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1] ] } ],
  "entities": [],
  "waves": [ { "name": "1", "streams": [] } ]
)json")),
		std::runtime_error);
}

TEST_CASE("stream missing name is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1] ] } ],
  "entities": [],
  "waves": [
    {
      "name": "1",
      "streams": [
        {
          "path": "main",
          "spawnInterval": 1,
          "spawns": [ { "id": "runner", "count": 1 } ]
        }
      ]
    }
  ]
)json")),
		std::runtime_error);
}

TEST_CASE("stream missing path is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1] ] } ],
  "entities": [],
  "waves": [
    {
      "name": "1",
      "streams": [
        {
          "name": "main",
          "spawnInterval": 1,
          "spawns": [ { "id": "runner", "count": 1 } ]
        }
      ]
    }
  ]
)json")),
		std::runtime_error);
}

TEST_CASE("stream missing spawns is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1] ] } ],
  "entities": [],
  "waves": [
    {
      "name": "1",
      "streams": [
        {
          "name": "main",
          "path": "main",
          "spawnInterval": 1
        }
      ]
    }
  ]
)json")),
		std::runtime_error);
}

TEST_CASE("empty spawns array is load fail", "[tower][paths][catalog]")
{
	tower::LevelCatalog level;
	CHECK_THROWS_AS(
		loadLevel(
			level,
			wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1] ] } ],
  "entities": [],
  "waves": [
    {
      "name": "1",
      "streams": [
        {
          "name": "main",
          "path": "main",
          "spawnInterval": 1,
          "spawns": []
        }
      ]
    }
  ]
)json")),
		std::runtime_error);
}

TEST_CASE("on-path entity placement loads", "[tower][paths][entity]")
{
	tower::LevelCatalog level;
	loadLevel(
		level,
		wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1], [0, 2] ] } ],
  "entities": [ { "id": "rocks", "x": 0, "z": 1 } ],
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
)json"));
	REQUIRE(level.entities().size() == 1);
	CHECK(level.entities()[0].z == 1);
}

TEST_CASE("off-path scenery still loads", "[tower][paths][entity]")
{
	tower::LevelCatalog level;
	loadLevel(
		level,
		wrap(R"json(
  "paths": [ { "name": "main", "path": [ [0, 0], [0, 1], [0, 2] ] } ],
  "entities": [ { "id": "tree", "x": 3, "z": 3 } ],
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
)json"));
	REQUIRE(level.entities().size() == 1);
	CHECK(level.entities()[0].id == "tree");
}
