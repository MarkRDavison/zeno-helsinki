#include <catch2/catch_test_macros.hpp>
#include <Commands/GameCommands.hpp>
#include <Components/HealthComponent.hpp>
#include <Components/TeamComponent.hpp>
#include <Components/TowerComponent.hpp>
#include <SceneCatalog.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <Services/GameCommandService.hpp>
#include <Services/GameStateService.hpp>
#include <Services/LevelCatalog.hpp>
#include <Services/MatchContext.hpp>
#include <Services/TowerBuildService.hpp>
#include <Services/TowerFocusService.hpp>
#include <Services/TowerSelectionService.hpp>
#include <Services/ProjectileCatalog.hpp>
#include <Services/StatusCatalog.hpp>
#include <Services/StatusCategoryCatalog.hpp>
#include <Services/TowerCatalog.hpp>
#include <Services/WaveService.hpp>
#include <Services/WeaponCatalog.hpp>
#include <Targeting.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
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

	constexpr auto kCategories = R"json(
[
  { "id": "slow", "kind": "stat", "color": [0, 0, 1] },
  { "id": "poison", "kind": "dot", "color": [0, 1, 0] },
  { "id": "burn", "kind": "dot", "color": [1, 0, 0] },
  { "id": "weakness", "kind": "stat", "color": [1, 0, 1] }
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

	constexpr auto kBolt = R"json(
[
  { "id": "bolt", "model": "marker", "damage": 1, "damageType": "physical", "speed": 6, "hitRadius": 0.35, "y": 0.4, "scale": [0.15, 0.15, 0.15] }
]
)json";

	constexpr auto kWeapons = R"json(
[
  { "id": "single_cannon", "projectile": "bolt", "fireCooldown": 0.7 }
]
)json";

	constexpr auto kTowers = R"json(
[
  { "id": "single", "model": "turret_single", "label": "Single", "cost": 15, "range": 3, "weapons": [ { "id": "single_cannon", "offset": [0, 0, 0] } ] },
  { "id": "double", "model": "turret_double", "label": "Double", "cost": 25, "range": 3.5, "weapons": [ { "id": "single_cannon", "offset": [0, 0, 0] } ] }
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
		tower::StatusCategoryCatalog categories;
		tower::StatusCatalog statuses;
		tower::ProjectileCatalog projectiles;
		tower::WeaponCatalog weapons;
		tower::TowerCatalog towers;
		tower::CreepCatalog creeps;
		tower::EntityCatalog entities;
		tower::LevelCatalog level;
		tower::MatchContext match;
		hl::Scene scene;
		tower::GameStateService gameState;
		tower::WaveService wave;
		tower::TowerSelectionService selection;
		tower::TowerFocusService focus;
		tower::TowerBuildService build;
		tower::GameCommandService commands;

		Fixture() :
			gameState((load(), level), match),
			wave(gameState, creeps, level),
			selection(scene, gameState),
			focus(scene, selection, gameState, towers, level),
			build(scene, wave, gameState, towers, level, match),
			commands(selection, focus, build)
		{
		}

		void load()
		{
			types.loadFromText(kTypes, "damage-types.json");
			categories.loadFromText(kCategories, "status-categories.json");
			statuses.loadFromText(kStatuses, "statuses.json", categories, types);
			projectiles.loadFromText(kBolt, "projectiles.json", types, statuses);
			weapons.loadFromText(kWeapons, "weapons.json", projectiles);
			towers.loadFromText(kTowers, "towers.json", weapons);
			creeps.loadFromText(kCreeps, "creeps.json", types);
			entities.loadFromText(
				R"json([ { "id": "rocks", "model": "rocks", "size": [1, 1] } ])json",
				"entities.json",
				types);
			level.loadFromText(kLevel, "level.json", creeps, entities);
		}

		hl::Entity* addTower()
		{
			auto* entity = scene.addEntity();
			entity->AddTag(tower::TowerTag);
			entity->AddComponent<tower::TeamComponent>()->team = tower::Team::Tower;
			auto* tower = entity->AddComponent<tower::TowerComponent>();
			tower->x = 2;
			tower->z = 2;
			tower->defId = "single";
			entity->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(2.5f, 0.0f, 2.5f));
			return entity;
		}

		hl::Entity* addNeutral(int tx, int tz, bool withHealth)
		{
			auto* entity = scene.addEntity();
			entity->AddComponent<tower::TeamComponent>()->team = tower::Team::Neutral;
			entity->AddComponent<hl::TransformComponent>()->SetPosition(level.tileCenter(tx, tz));
			if (withHealth)
			{
				auto* health = entity->AddComponent<tower::HealthComponent>();
				health->max = 8.0f;
				health->current = 8.0f;
			}
			return entity;
		}

		int liveTowerCount() const
		{
			int count = 0;
			for (auto* entity : scene.getEntitiesByTag(tower::TowerTag))
			{
				if (!scene.isPendingRemoval(entity->Id))
				{
					++count;
				}
			}
			return count;
		}

		bool hasTowerAt(int x, int z) const
		{
			for (auto* entity : scene.getEntitiesByTag(tower::TowerTag))
			{
				if (scene.isPendingRemoval(entity->Id))
				{
					continue;
				}

				const auto* tower = entity->GetComponent<tower::TowerComponent>();
				if (tower != nullptr && tower->x == x && tower->z == z)
				{
					return true;
				}
			}
			return false;
		}
	};
}

TEST_CASE("AssignTowerFocus is false with no selection", "[tower][command]")
{
	Fixture fixture;
	auto* rock = fixture.addNeutral(2, 3, true);
	CHECK_FALSE(fixture.commands.handle(tower::AssignTowerFocus{ .targetEntityId = rock->Id }));
}

TEST_CASE("AssignTowerFocus succeeds in build when Neutral is in range", "[tower][command]")
{
	Fixture fixture;
	auto* gun = fixture.addTower();
	auto* rock = fixture.addNeutral(2, 3, true);
	REQUIRE(fixture.commands.handle(tower::SelectTower{ .towerEntityId = gun->Id }));
	CHECK_FALSE(fixture.wave.inCombat());
	REQUIRE(fixture.commands.handle(tower::AssignTowerFocus{ .targetEntityId = rock->Id }));
	CHECK(gun->GetComponent<tower::TowerComponent>()->focusEntityId == rock->Id);
}

TEST_CASE("AssignTowerFocus ignores immortal and keeps previous focus", "[tower][command]")
{
	Fixture fixture;
	auto* gun = fixture.addTower();
	auto* rock = fixture.addNeutral(2, 3, true);
	auto* immortal = fixture.addNeutral(3, 2, false);
	REQUIRE(fixture.commands.handle(tower::SelectTower{ .towerEntityId = gun->Id }));
	REQUIRE(fixture.commands.handle(tower::AssignTowerFocus{ .targetEntityId = rock->Id }));
	CHECK_FALSE(fixture.commands.handle(tower::AssignTowerFocus{ .targetEntityId = immortal->Id }));
	CHECK(gun->GetComponent<tower::TowerComponent>()->focusEntityId == rock->Id);
}

TEST_CASE("AssignTowerFocus ignores out of range and keeps previous focus", "[tower][command]")
{
	Fixture fixture;
	auto* gun = fixture.addTower();
	auto* rock = fixture.addNeutral(2, 3, true);
	auto* far = fixture.addNeutral(7, 7, true);
	REQUIRE(fixture.commands.handle(tower::SelectTower{ .towerEntityId = gun->Id }));
	REQUIRE(fixture.commands.handle(tower::AssignTowerFocus{ .targetEntityId = rock->Id }));
	CHECK_FALSE(fixture.commands.handle(tower::AssignTowerFocus{ .targetEntityId = far->Id }));
	CHECK(gun->GetComponent<tower::TowerComponent>()->focusEntityId == rock->Id);
}

TEST_CASE("PlaceTower is false while in combat", "[tower][command]")
{
	Fixture fixture;
	REQUIRE(fixture.wave.tryStart());
	REQUIRE(fixture.wave.inCombat());
	const int gold = fixture.gameState.gold();
	CHECK_FALSE(fixture.commands.handle(tower::PlaceTower{ .defId = "single", .x = 3, .z = 3 }));
	CHECK(fixture.gameState.gold() == gold);
	CHECK(fixture.liveTowerCount() == 0);
}

TEST_CASE("PlaceTower spends gold and spawns a tower", "[tower][command]")
{
	Fixture fixture;
	const int gold = fixture.gameState.gold();
	REQUIRE(fixture.commands.handle(tower::PlaceTower{ .defId = "single", .x = 3, .z = 3 }));
	CHECK(fixture.gameState.gold() == gold - 15);
	CHECK(fixture.hasTowerAt(3, 3));
	CHECK(fixture.liveTowerCount() == 1);
}

TEST_CASE("PlaceTower is false on a path tile", "[tower][command]")
{
	Fixture fixture;
	const int gold = fixture.gameState.gold();
	CHECK_FALSE(fixture.commands.handle(tower::PlaceTower{ .defId = "single", .x = 0, .z = 0 }));
	CHECK(fixture.gameState.gold() == gold);
	CHECK(fixture.liveTowerCount() == 0);
}

TEST_CASE("PlaceTower is false on an occupied tile", "[tower][command]")
{
	Fixture fixture;
	fixture.addTower();
	const int gold = fixture.gameState.gold();
	CHECK_FALSE(fixture.commands.handle(tower::PlaceTower{ .defId = "single", .x = 2, .z = 2 }));
	CHECK(fixture.gameState.gold() == gold);
	CHECK(fixture.liveTowerCount() == 1);
}

TEST_CASE("PlaceTower is false when too poor", "[tower][command]")
{
	Fixture fixture;
	while (fixture.gameState.gold() >= 15)
	{
		REQUIRE(fixture.gameState.trySpend(15));
	}
	const int gold = fixture.gameState.gold();
	CHECK_FALSE(fixture.commands.handle(tower::PlaceTower{ .defId = "single", .x = 3, .z = 3 }));
	CHECK(fixture.gameState.gold() == gold);
	CHECK(fixture.liveTowerCount() == 0);
}

TEST_CASE("campaign PlaceTower refuses unowned ids", "[tower][command]")
{
	Fixture fixture;
	fixture.match.campaign = true;
	fixture.match.ownedTowers = { "single" };
	const int gold = fixture.gameState.gold();
	CHECK_FALSE(fixture.commands.handle(tower::PlaceTower{ .defId = "double", .x = 3, .z = 3 }));
	CHECK(fixture.gameState.gold() == gold);
	CHECK(fixture.liveTowerCount() == 0);
	REQUIRE(fixture.commands.handle(tower::PlaceTower{ .defId = "single", .x = 3, .z = 3 }));
	CHECK(fixture.hasTowerAt(3, 3));
}

TEST_CASE("skirmish PlaceTower allows catalog ids not in ownedTowers", "[tower][command]")
{
	Fixture fixture;
	REQUIRE_FALSE(fixture.match.campaign);
	REQUIRE(fixture.match.ownedTowers.empty());
	REQUIRE(fixture.commands.handle(tower::PlaceTower{ .defId = "double", .x = 4, .z = 4 }));
	CHECK(fixture.hasTowerAt(4, 4));
}
