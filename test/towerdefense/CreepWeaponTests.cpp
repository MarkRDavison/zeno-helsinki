#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Combat.hpp>
#include <Components/EntityComponent.hpp>
#include <Components/HealthComponent.hpp>
#include <Components/ProjectileComponent.hpp>
#include <Components/TeamComponent.hpp>
#include <Armed.hpp>
#include <SceneCatalog.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/ProjectileCatalog.hpp>
#include <Services/StatusCatalog.hpp>
#include <Services/StatusCategoryCatalog.hpp>
#include <Services/WeaponCatalog.hpp>
#include <Systems/ProjectileSystem.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
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
  { "id": "bolt", "model": "marker", "damage": 4, "damageType": "physical", "speed": 6, "hitRadius": 0.35, "y": 0.4, "scale": [0.15, 0.15, 0.15] }
]
)json";

	constexpr auto kWeapons = R"json(
[
  { "id": "single_cannon", "projectile": "bolt", "fireCooldown": 0.7 }
]
)json";

	tower::DamageTypeCatalog types()
	{
		tower::DamageTypeCatalog catalog;
		catalog.loadFromText(kTypes, "damage-types.json");
		return catalog;
	}

	struct CombatCatalogs
	{
		tower::DamageTypeCatalog types;
		tower::StatusCategoryCatalog categories;
		tower::StatusCatalog statuses;
		tower::ProjectileCatalog projectiles;
		tower::WeaponCatalog weapons;

		CombatCatalogs()
		{
			types.loadFromText(kTypes, "damage-types.json");
			categories.loadFromText(kCategories, "status-categories.json");
			statuses.loadFromText(kStatuses, "statuses.json", categories, types);
			projectiles.loadFromText(kBolt, "projectiles.json", types, statuses);
			weapons.loadFromText(kWeapons, "weapons.json", projectiles);
		}
	};
}

TEST_CASE("omitted slots is unarmed", "[tower][creep][slots]")
{
	const auto loaded = types();
	tower::CreepCatalog creeps;
	creeps.loadFromText(
		R"json([
  { "id": "runner", "model": "creep_runner", "health": 2, "speed": 2.2 },
  { "id": "tank", "model": "creep_tank", "health": 8, "speed": 0.85 }
])json",
		"creeps.json",
		loaded);
	CHECK(creeps.find("runner")->slots.empty());
	CHECK_FALSE(tower::canFireAtStalled(creeps.find("runner")->slots, nullptr));
}

TEST_CASE("empty slots is unarmed", "[tower][creep][slots]")
{
	const auto loaded = types();
	tower::CreepCatalog creeps;
	creeps.loadFromText(
		R"json([
  { "id": "runner", "model": "creep_runner", "health": 2, "speed": 2.2, "slots": [] },
  { "id": "tank", "model": "creep_tank", "health": 8, "speed": 0.85 }
])json",
		"creeps.json",
		loaded);
	CHECK(creeps.find("runner")->slots.empty());
}

TEST_CASE("unknown slot weapon is load fail", "[tower][creep][slots]")
{
	const auto loaded = types();
	tower::CreepCatalog creeps;
	CHECK_THROWS_AS(
		creeps.loadFromText(
			R"json([
  { "id": "runner", "model": "creep_runner", "health": 2, "speed": 2.2, "slots": [ { "id": "teeth", "offset": [0, 0, 0] } ] },
  { "id": "tank", "model": "creep_tank", "health": 8, "speed": 0.85 }
])json",
			"creeps.json",
			loaded),
		std::runtime_error);
}

TEST_CASE("creep can slot shared single_cannon", "[tower][creep][slots]")
{
	CombatCatalogs catalogs;
	tower::CreepCatalog creeps;
	creeps.loadFromText(
		R"json([
  { "id": "runner", "model": "creep_runner", "health": 2, "speed": 2.2 },
  { "id": "tank", "model": "creep_tank", "health": 8, "speed": 0.85, "slots": [ { "id": "single_cannon", "offset": [0.2, 0.4, 0.3] } ] }
])json",
		"creeps.json",
		catalogs.types,
		catalogs.weapons);
	REQUIRE(creeps.find("tank")->slots.size() == 1);
	CHECK(creeps.find("tank")->slots[0].id == "single_cannon");
	CHECK(catalogs.weapons.find("single_cannon") != nullptr);
}

TEST_CASE("unarmed does not fire at a path entity", "[tower][creep][fire]")
{
	hl::Entity rock(1);
	rock.AddComponent<tower::HealthComponent>()->current = 8.0f;
	CHECK_FALSE(tower::canFireAtStalled({}, &rock));
}

TEST_CASE("slotted shot damages a Neutral entity", "[tower][creep][fire]")
{
	CombatCatalogs catalogs;
	hl::Scene scene;
	auto* rock = scene.addEntity();
	rock->AddComponent<tower::TeamComponent>()->team = tower::Team::Neutral;
	rock->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	auto* health = rock->AddComponent<tower::HealthComponent>();
	health->max = 8.0f;
	health->current = 8.0f;
	rock->AddComponent<tower::EntityComponent>()->resist["physical"] = 0.0f;

	auto* shot = scene.addEntity();
	shot->AddTag(tower::ProjectileTag);
	shot->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.0f, 0.4f, 0.0f));
	auto* projectile = shot->AddComponent<tower::ProjectileComponent>();
	projectile->targetId = rock->Id;
	projectile->speed = 6.0f;
	projectile->damage = 4.0f;
	projectile->damageType = "physical";
	projectile->hitRadius = 0.35f;
	projectile->y = 0.4f;
	projectile->lastDest = glm::vec3(0.0f, 0.4f, 0.0f);

	bool gold = false;
	tower::ProjectileSystem system(scene, catalogs.statuses, catalogs.categories);
	system.onKill = [&]() { gold = true; };
	system.update(0.1f);
	CHECK(health->current == Catch::Approx(4.0f));
	CHECK_FALSE(gold);
	CHECK_FALSE(tower::isDead(health->current));
}

TEST_CASE("entity dies at current <= 0 with no gold", "[tower][creep][fire]")
{
	CombatCatalogs catalogs;
	hl::Scene scene;
	auto* rock = scene.addEntity();
	rock->AddComponent<tower::TeamComponent>()->team = tower::Team::Neutral;
	rock->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	auto* health = rock->AddComponent<tower::HealthComponent>();
	health->max = 4.0f;
	health->current = 4.0f;

	auto* shot = scene.addEntity();
	shot->AddTag(tower::ProjectileTag);
	shot->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.0f, 0.4f, 0.0f));
	auto* projectile = shot->AddComponent<tower::ProjectileComponent>();
	projectile->targetId = rock->Id;
	projectile->speed = 6.0f;
	projectile->damage = 4.0f;
	projectile->damageType = "physical";
	projectile->hitRadius = 0.35f;
	projectile->y = 0.4f;
	projectile->lastDest = glm::vec3(0.0f, 0.4f, 0.0f);

	bool gold = false;
	tower::ProjectileSystem system(scene, catalogs.statuses, catalogs.categories);
	system.onKill = [&]() { gold = true; };
	system.update(0.1f);
	CHECK(tower::isDead(health->current));
	CHECK_FALSE(gold);
	CHECK(scene.isPendingRemoval(rock->Id));
}

TEST_CASE("immortal entity hit does not remove", "[tower][creep][fire]")
{
	CombatCatalogs catalogs;
	hl::Scene scene;
	auto* rock = scene.addEntity();
	rock->AddComponent<tower::TeamComponent>()->team = tower::Team::Neutral;
	rock->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

	auto* shot = scene.addEntity();
	shot->AddTag(tower::ProjectileTag);
	shot->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.0f, 0.4f, 0.0f));
	auto* projectile = shot->AddComponent<tower::ProjectileComponent>();
	projectile->targetId = rock->Id;
	projectile->speed = 6.0f;
	projectile->damage = 4.0f;
	projectile->damageType = "physical";
	projectile->hitRadius = 0.35f;
	projectile->y = 0.4f;
	projectile->lastDest = glm::vec3(0.0f, 0.4f, 0.0f);

	bool gold = false;
	tower::ProjectileSystem system(scene, catalogs.statuses, catalogs.categories);
	system.onKill = [&]() { gold = true; };
	system.update(0.1f);
	CHECK_FALSE(gold);
	CHECK_FALSE(scene.isPendingRemoval(rock->Id));
}
