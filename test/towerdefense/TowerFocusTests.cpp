#include <catch2/catch_test_macros.hpp>
#include <Components/HealthComponent.hpp>
#include <Components/TeamComponent.hpp>
#include <Targeting.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>

TEST_CASE("no focus prefers Creep over closer Neutral", "[tower][focus]")
{
	hl::Scene scene;
	auto* creep = scene.addEntity();
	creep->AddComponent<tower::TeamComponent>()->team = tower::Team::Creep;
	creep->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(2.0f, 0.0f, 0.0f));

	auto* rock = scene.addEntity();
	rock->AddComponent<tower::TeamComponent>()->team = tower::Team::Neutral;
	rock->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.5f, 0.0f, 0.0f));
	rock->AddComponent<tower::HealthComponent>()->current = 8.0f;

	int focus = tower::TowerFocusNone;
	const glm::vec3 from{ 0.0f, 0.0f, 0.0f };
	CHECK(tower::resolveTowerFireTarget(scene, from, 3.0f, focus) == creep);
	CHECK(focus == tower::TowerFocusNone);
}

TEST_CASE("RMB focus Neutral in range is chosen over Creep", "[tower][focus]")
{
	hl::Scene scene;
	auto* creep = scene.addEntity();
	creep->AddComponent<tower::TeamComponent>()->team = tower::Team::Creep;
	creep->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.4f, 0.0f, 0.0f));

	auto* rock = scene.addEntity();
	rock->AddComponent<tower::TeamComponent>()->team = tower::Team::Neutral;
	rock->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(1.5f, 0.0f, 0.0f));
	rock->AddComponent<tower::HealthComponent>()->current = 8.0f;

	int focus = tower::TowerFocusNone;
	const glm::vec3 from{ 0.0f, 0.0f, 0.0f };
	REQUIRE(tower::tryAssignTowerFocus(scene, from, 3.0f, rock, focus));
	CHECK(focus == rock->Id);
	CHECK(tower::resolveTowerFireTarget(scene, from, 3.0f, focus) == rock);
}

TEST_CASE("immortal Neutral is ignored and previous focus kept", "[tower][focus]")
{
	hl::Scene scene;
	auto* focused = scene.addEntity();
	focused->AddComponent<tower::TeamComponent>()->team = tower::Team::Neutral;
	focused->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(1.0f, 0.0f, 0.0f));
	focused->AddComponent<tower::HealthComponent>()->current = 8.0f;

	auto* immortal = scene.addEntity();
	immortal->AddComponent<tower::TeamComponent>()->team = tower::Team::Neutral;
	immortal->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.5f, 0.0f, 0.0f));

	int focus = tower::TowerFocusNone;
	const glm::vec3 from{ 0.0f, 0.0f, 0.0f };
	REQUIRE(tower::tryAssignTowerFocus(scene, from, 3.0f, focused, focus));
	CHECK_FALSE(tower::tryAssignTowerFocus(scene, from, 3.0f, immortal, focus));
	CHECK(focus == focused->Id);
}

TEST_CASE("out of range Neutral is ignored and previous focus kept", "[tower][focus]")
{
	hl::Scene scene;
	auto* focused = scene.addEntity();
	focused->AddComponent<tower::TeamComponent>()->team = tower::Team::Neutral;
	focused->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(1.0f, 0.0f, 0.0f));
	focused->AddComponent<tower::HealthComponent>()->current = 8.0f;

	auto* far = scene.addEntity();
	far->AddComponent<tower::TeamComponent>()->team = tower::Team::Neutral;
	far->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(10.0f, 0.0f, 0.0f));
	far->AddComponent<tower::HealthComponent>()->current = 8.0f;

	int focus = tower::TowerFocusNone;
	const glm::vec3 from{ 0.0f, 0.0f, 0.0f };
	REQUIRE(tower::tryAssignTowerFocus(scene, from, 3.0f, focused, focus));
	CHECK_FALSE(tower::tryAssignTowerFocus(scene, from, 3.0f, far, focus));
	CHECK(focus == focused->Id);
}

TEST_CASE("clearing focus restores Creep", "[tower][focus]")
{
	hl::Scene scene;
	auto* creep = scene.addEntity();
	creep->AddComponent<tower::TeamComponent>()->team = tower::Team::Creep;
	creep->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(1.0f, 0.0f, 0.0f));

	auto* rock = scene.addEntity();
	rock->AddComponent<tower::TeamComponent>()->team = tower::Team::Neutral;
	rock->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.5f, 0.0f, 0.0f));
	rock->AddComponent<tower::HealthComponent>()->current = 8.0f;

	int focus = tower::TowerFocusNone;
	const glm::vec3 from{ 0.0f, 0.0f, 0.0f };
	REQUIRE(tower::tryAssignTowerFocus(scene, from, 3.0f, rock, focus));
	REQUIRE(tower::resolveTowerFireTarget(scene, from, 3.0f, focus) == rock);

	focus = tower::TowerFocusNone;
	CHECK(tower::resolveTowerFireTarget(scene, from, 3.0f, focus) == creep);
	CHECK(focus == tower::TowerFocusNone);
}

TEST_CASE("stale focus from death restores Creep", "[tower][focus]")
{
	hl::Scene scene;
	auto* creep = scene.addEntity();
	creep->AddComponent<tower::TeamComponent>()->team = tower::Team::Creep;
	creep->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(1.0f, 0.0f, 0.0f));

	auto* rock = scene.addEntity();
	rock->AddComponent<tower::TeamComponent>()->team = tower::Team::Neutral;
	rock->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.5f, 0.0f, 0.0f));
	rock->AddComponent<tower::HealthComponent>()->current = 8.0f;

	int focus = tower::TowerFocusNone;
	const glm::vec3 from{ 0.0f, 0.0f, 0.0f };
	REQUIRE(tower::tryAssignTowerFocus(scene, from, 3.0f, rock, focus));
	scene.removeEntity(rock->Id);

	CHECK(tower::resolveTowerFireTarget(scene, from, 3.0f, focus) == creep);
	CHECK(focus == tower::TowerFocusNone);
}

TEST_CASE("stale focus from leaving range restores Creep", "[tower][focus]")
{
	hl::Scene scene;
	auto* creep = scene.addEntity();
	creep->AddComponent<tower::TeamComponent>()->team = tower::Team::Creep;
	creep->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(1.0f, 0.0f, 0.0f));

	auto* rock = scene.addEntity();
	rock->AddComponent<tower::TeamComponent>()->team = tower::Team::Neutral;
	rock->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.5f, 0.0f, 0.0f));
	rock->AddComponent<tower::HealthComponent>()->current = 8.0f;

	int focus = tower::TowerFocusNone;
	const glm::vec3 from{ 0.0f, 0.0f, 0.0f };
	REQUIRE(tower::tryAssignTowerFocus(scene, from, 3.0f, rock, focus));
	rock->GetComponent<hl::TransformComponent>()->SetPosition(glm::vec3(10.0f, 0.0f, 0.0f));

	CHECK(tower::resolveTowerFireTarget(scene, from, 3.0f, focus) == creep);
	CHECK(focus == tower::TowerFocusNone);
}
