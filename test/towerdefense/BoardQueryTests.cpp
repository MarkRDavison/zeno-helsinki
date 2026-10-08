#include <catch2/catch_test_macros.hpp>
#include <BoardQuery.hpp>
#include <Components/EntityComponent.hpp>
#include <Components/TeamComponent.hpp>
#include <Components/TowerComponent.hpp>
#include <SceneCatalog.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <Services/LevelCatalog.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>

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

	constexpr auto kLevel = R"json(
{
  "id": "test",
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
		hl::Scene scene;

		Fixture()
		{
			types.loadFromText(kTypes, "damage-types.json");
			creeps.loadFromText(kCreeps, "creeps.json", types);
			entities.loadFromText(
				R"json([ { "id": "rocks", "model": "rocks", "size": [1, 1] } ])json",
				"entities.json",
				types);
			level.loadFromText(kLevel, "level.json", creeps, entities);
		}

		void addTower(int x, int z)
		{
			auto* entity = scene.addEntity();
			entity->AddTag(tower::TowerTag);
			entity->AddComponent<tower::TeamComponent>()->team = tower::Team::Tower;
			auto* tower = entity->AddComponent<tower::TowerComponent>();
			tower->x = x;
			tower->z = z;
			tower->defId = "single";
		}

		void addEntityFootprint(int x, int z, int sizeX, int sizeZ)
		{
			auto* entity = scene.addEntity();
			entity->AddComponent<tower::TeamComponent>()->team = tower::Team::Neutral;
			auto* placed = entity->AddComponent<tower::EntityComponent>();
			placed->x = x;
			placed->z = z;
			placed->sizeX = sizeX;
			placed->sizeZ = sizeZ;
		}
	};
}

TEST_CASE("empty buildable tile is not occupied or unbuildable", "[tower][board]")
{
	Fixture fixture;
	CHECK_FALSE(tower::tileOccupied(fixture.scene, 3, 3));
	CHECK_FALSE(tower::tileUnbuildable(fixture.level, fixture.scene, 3, 3));
}

TEST_CASE("path tile is unbuildable", "[tower][board]")
{
	Fixture fixture;
	CHECK_FALSE(tower::tileOccupied(fixture.scene, 0, 0));
	CHECK(tower::tileUnbuildable(fixture.level, fixture.scene, 0, 0));
}

TEST_CASE("tower occupies its tile only", "[tower][board]")
{
	Fixture fixture;
	fixture.addTower(2, 2);
	CHECK(tower::tileOccupied(fixture.scene, 2, 2));
	CHECK(tower::tileUnbuildable(fixture.level, fixture.scene, 2, 2));
	CHECK_FALSE(tower::tileOccupied(fixture.scene, 3, 2));
	CHECK_FALSE(tower::tileUnbuildable(fixture.level, fixture.scene, 3, 2));
}

TEST_CASE("entity footprint occupies every covered tile", "[tower][board]")
{
	Fixture fixture;
	fixture.addEntityFootprint(3, 3, 2, 1);
	CHECK(tower::tileOccupied(fixture.scene, 3, 3));
	CHECK(tower::tileOccupied(fixture.scene, 4, 3));
	CHECK(tower::tileUnbuildable(fixture.level, fixture.scene, 3, 3));
	CHECK(tower::tileUnbuildable(fixture.level, fixture.scene, 4, 3));
	CHECK_FALSE(tower::tileOccupied(fixture.scene, 5, 3));
	CHECK_FALSE(tower::tileUnbuildable(fixture.level, fixture.scene, 5, 3));
}

TEST_CASE("off-board tile is unbuildable", "[tower][board]")
{
	Fixture fixture;
	CHECK(tower::tileUnbuildable(fixture.level, fixture.scene, -1, 0));
	CHECK(tower::tileUnbuildable(fixture.level, fixture.scene, 8, 0));
	CHECK(tower::tileUnbuildable(fixture.level, fixture.scene, 0, 8));
}
