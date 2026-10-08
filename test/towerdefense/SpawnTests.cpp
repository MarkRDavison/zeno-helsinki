#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Components/CreepComponent.hpp>
#include <Components/EntityComponent.hpp>
#include <Components/HealthComponent.hpp>
#include <Components/PathFollowComponent.hpp>
#include <Components/StatusListComponent.hpp>
#include <Components/TeamComponent.hpp>
#include <Components/TowerComponent.hpp>
#include <SceneCatalog.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <Services/LevelCatalog.hpp>
#include <Services/TowerCatalog.hpp>
#include <Spawn.hpp>
#include <TestCatalogJson.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>

namespace
{
	constexpr auto kEntities = R"json(
[
  { "id": "rocks", "model": "rocks", "size": [1, 1] }
]
)json";

	constexpr auto kLevel = R"json(
{
  "id": "spawn-test",
  "boardSize": [8, 8],
  "startGold": 100,
  "startLives": 3,
  "killGold": 10,
  "waveClearBonus": 25,
  "buildTimer": 20,
  "paths": [
    { "name": "main", "path": [ [0, 0], [0, 1] ] }
  ],
  "entities": [
    { "id": "rocks", "x": 2, "z": 2 }
  ],
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
			types.loadFromText(tower::test::kTypes, "damage-types.json");
			creeps.loadFromText(tower::test::kCreeps, "creeps.json", types);
			entities.loadFromText(kEntities, "entities.json", types);
			level.loadFromText(kLevel, "level.json", creeps, entities);
		}
	};
}

TEST_CASE("spawnCreep adds StatusList on Team Creep", "[tower][spawn]")
{
	Fixture f;
	const auto* def = f.creeps.find("runner");
	auto* entity = tower::spawnCreep(f.scene, *def, f.level, "main");
	CHECK(hasTeam(entity, tower::Team::Creep));
	CHECK(entity->HasTag(tower::CreepTag));
	REQUIRE(entity->GetComponent<tower::StatusListComponent>() != nullptr);
	CHECK(entity->GetComponent<tower::HealthComponent>()->max == Catch::Approx(def->health));
	CHECK(entity->GetComponent<tower::PathFollowComponent>()->speed == Catch::Approx(def->speed));
	CHECK(entity->GetComponent<tower::CreepComponent>()->baseHealth == Catch::Approx(def->health));
}

TEST_CASE("spawnTower does not add StatusList", "[tower][spawn]")
{
	Fixture f;
	tower::TowerDef def;
	def.id = "single";
	def.model = "turret_single";
	auto* entity = tower::spawnTower(f.scene, def, f.level, 1, 0);
	CHECK(entity->HasTag(tower::TowerTag));
	CHECK(hasTeam(entity, tower::Team::Tower));
	CHECK(entity->GetComponent<tower::TowerComponent>() != nullptr);
	CHECK(entity->GetComponent<tower::StatusListComponent>() == nullptr);
}

TEST_CASE("spawnEntities does not add StatusList", "[tower][spawn]")
{
	Fixture f;
	tower::spawnEntities(f.scene, f.level, f.entities);
	auto placed = f.scene.getEntitiesWithComponents<tower::EntityComponent>();
	REQUIRE(placed.size() == 1);
	CHECK(hasTeam(placed[0], tower::Team::Neutral));
	CHECK(placed[0]->GetComponent<tower::StatusListComponent>() == nullptr);
}
