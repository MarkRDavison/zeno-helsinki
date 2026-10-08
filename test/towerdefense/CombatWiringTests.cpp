#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Components/CreepComponent.hpp>
#include <Components/HealthComponent.hpp>
#include <Components/PathFollowComponent.hpp>
#include <Components/ProjectileComponent.hpp>
#include <Components/StatusListComponent.hpp>
#include <Components/TeamComponent.hpp>
#include <SceneCatalog.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/StatusCatalog.hpp>
#include <Services/StatusCategoryCatalog.hpp>
#include <Status.hpp>
#include <Systems/ProjectileSystem.hpp>
#include <Systems/StatusSystem.hpp>
#include <TestCatalogJson.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>

namespace
{
	constexpr auto kCategories = R"json(
[
  { "id": "slow", "kind": "stat", "color": [0, 0, 1] },
  { "id": "poison", "kind": "dot", "color": [0, 1, 0] },
  { "id": "burn", "kind": "dot", "color": [1, 0, 0] },
  { "id": "weakness", "kind": "stat", "color": [1, 0, 1] },
  { "id": "health", "kind": "stat", "color": [1, 1, 0], "cap": 2 },
  { "id": "resist", "kind": "stat", "color": [0.5, 0.5, 0.5] }
]
)json";

	constexpr auto kStatuses = R"json(
[
  { "id": "slow", "category": "slow", "duration": 3, "magnitude": 0.5, "channel": "speed" },
  { "id": "poison", "category": "poison", "duration": 2.5, "interval": 1, "tickDamage": 10, "damageType": "poison" },
  { "id": "burn", "category": "burn", "duration": 2.5, "interval": 1, "tickDamage": 1, "damageType": "fire" },
  { "id": "weakness", "category": "weakness", "duration": 3, "magnitude": 0.2, "channel": "weakness" },
  { "id": "vitality", "category": "health", "duration": 1, "magnitude": 0.1, "channel": "health" },
  { "id": "iron-skin", "category": "resist", "duration": 3, "magnitude": 0.2, "channel": "physical_resistance" }
]
)json";

	struct Catalogs
	{
		tower::DamageTypeCatalog types;
		tower::StatusCategoryCatalog categories;
		tower::StatusCatalog statuses;

		Catalogs()
		{
			types.loadFromText(tower::test::kTypes, "damage-types.json");
			categories.loadFromText(kCategories, "status-categories.json");
			statuses.loadFromText(kStatuses, "statuses.json", categories, types);
		}
	};

	void applyDef(
		tower::StatusListComponent& list,
		const Catalogs& catalogs,
		const char* id)
	{
		const auto* def = catalogs.statuses.find(id);
		const auto* category = catalogs.categories.find(def->category);
		tower::applyStatus(list.instances, *def, *category, catalogs.statuses, list.nextSeq);
	}

	hl::Entity* addCreep(hl::Scene& scene, float health, float physicalResist)
	{
		auto* creep = scene.addEntity();
		creep->AddComponent<tower::TeamComponent>()->team = tower::Team::Creep;
		creep->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		auto* hp = creep->AddComponent<tower::HealthComponent>();
		hp->max = health;
		hp->current = health;
		auto* body = creep->AddComponent<tower::CreepComponent>();
		body->baseHealth = health;
		body->resist["physical"] = physicalResist;
		creep->AddComponent<tower::StatusListComponent>();
		auto* follow = creep->AddComponent<tower::PathFollowComponent>();
		follow->baseSpeed = 2.0f;
		follow->speed = 2.0f;
		return creep;
	}

	void addShot(
		hl::Scene& scene,
		int targetId,
		float damage,
		std::vector<std::string> statuses = {})
	{
		auto* shot = scene.addEntity();
		shot->AddTag(tower::ProjectileTag);
		shot->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.0f, 0.4f, 0.0f));
		auto* projectile = shot->AddComponent<tower::ProjectileComponent>();
		projectile->targetId = targetId;
		projectile->speed = 6.0f;
		projectile->damage = damage;
		projectile->damageType = "physical";
		projectile->hitRadius = 0.35f;
		projectile->y = 0.4f;
		projectile->lastDest = glm::vec3(0.0f, 0.4f, 0.0f);
		projectile->statuses = std::move(statuses);
	}
}

TEST_CASE("ProjectileSystem hits a creep with innate physical resist", "[tower][combat][wiring]")
{
	Catalogs catalogs;
	hl::Scene scene;
	auto* creep = addCreep(scene, 10.0f, 0.25f);
	addShot(scene, creep->Id, 10.0f);
	bool killed = false;
	tower::ProjectileSystem system(scene, catalogs.statuses, catalogs.categories);
	system.onKill = [&]() { killed = true; };
	system.update(0.1f);
	CHECK(creep->GetComponent<tower::HealthComponent>()->current == Catch::Approx(2.5f));
	CHECK_FALSE(killed);
}

TEST_CASE("ProjectileSystem hit uses status weakness on the creep", "[tower][combat][wiring]")
{
	Catalogs catalogs;
	hl::Scene scene;
	auto* creep = addCreep(scene, 20.0f, 0.0f);
	applyDef(*creep->GetComponent<tower::StatusListComponent>(), catalogs, "weakness");
	addShot(scene, creep->Id, 10.0f);
	tower::ProjectileSystem system(scene, catalogs.statuses, catalogs.categories);
	system.update(0.1f);
	CHECK(creep->GetComponent<tower::HealthComponent>()->current == Catch::Approx(8.0f));
}

TEST_CASE("ProjectileSystem applies shot statuses to the creep", "[tower][combat][wiring]")
{
	Catalogs catalogs;
	hl::Scene scene;
	auto* creep = addCreep(scene, 10.0f, 0.0f);
	addShot(scene, creep->Id, 1.0f, { "slow" });
	tower::ProjectileSystem system(scene, catalogs.statuses, catalogs.categories);
	system.update(0.1f);
	const auto& list = creep->GetComponent<tower::StatusListComponent>()->instances;
	REQUIRE(list.size() == 1);
	CHECK(list[0].defId == "slow");
}

TEST_CASE("StatusSystem DoT kill fires onKill", "[tower][combat][wiring]")
{
	Catalogs catalogs;
	hl::Scene scene;
	auto* creep = addCreep(scene, 5.0f, 0.0f);
	applyDef(*creep->GetComponent<tower::StatusListComponent>(), catalogs, "poison");
	int gold = 0;
	tower::StatusSystem system(scene, catalogs.statuses);
	system.onKill = [&]() { gold += 1; };
	system.update(1.0f);
	CHECK(gold == 1);
	CHECK(scene.isPendingRemoval(creep->Id));
}

TEST_CASE("StatusSystem slow writes PathFollow speed", "[tower][combat][wiring]")
{
	Catalogs catalogs;
	hl::Scene scene;
	auto* creep = addCreep(scene, 10.0f, 0.0f);
	applyDef(*creep->GetComponent<tower::StatusListComponent>(), catalogs, "slow");
	tower::StatusSystem system(scene, catalogs.statuses);
	system.update(0.0f);
	const auto* follow = creep->GetComponent<tower::PathFollowComponent>();
	CHECK(follow->speed == Catch::Approx(follow->baseSpeed * 0.5f));
}

TEST_CASE("StatusSystem health expire clamps current to base max", "[tower][combat][wiring]")
{
	Catalogs catalogs;
	hl::Scene scene;
	auto* creep = addCreep(scene, 100.0f, 0.0f);
	applyDef(*creep->GetComponent<tower::StatusListComponent>(), catalogs, "vitality");
	tower::StatusSystem system(scene, catalogs.statuses);
	system.update(0.0f);
	auto* health = creep->GetComponent<tower::HealthComponent>();
	CHECK(health->max == Catch::Approx(110.0f));
	health->current = 110.0f;
	system.update(1.0f);
	CHECK(health->max == Catch::Approx(100.0f));
	CHECK(health->current == Catch::Approx(100.0f));
}

TEST_CASE("ProjectileSystem resist above 1 takes no damage", "[tower][combat][wiring]")
{
	Catalogs catalogs;
	hl::Scene scene;
	auto* creep = addCreep(scene, 10.0f, 0.9f);
	applyDef(*creep->GetComponent<tower::StatusListComponent>(), catalogs, "iron-skin");
	addShot(scene, creep->Id, 10.0f);
	tower::ProjectileSystem system(scene, catalogs.statuses, catalogs.categories);
	system.update(0.1f);
	CHECK(creep->GetComponent<tower::HealthComponent>()->current == Catch::Approx(10.0f));
}
