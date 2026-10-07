#include <catch2/catch_test_macros.hpp>
#include <Team.hpp>
#include <Targeting.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>

TEST_CASE("Team enum round-trips through int", "[tower][team]")
{
	CHECK(static_cast<tower::Team>(static_cast<int>(tower::Team::Tower)) == tower::Team::Tower);
	CHECK(static_cast<tower::Team>(static_cast<int>(tower::Team::Creep)) == tower::Team::Creep);
	CHECK(static_cast<tower::Team>(static_cast<int>(tower::Team::Neutral)) == tower::Team::Neutral);
}

TEST_CASE("creep is not Neutral", "[tower][team]")
{
	CHECK(tower::Team::Creep != tower::Team::Neutral);
	CHECK(tower::Team::Tower != tower::Team::Creep);
}

TEST_CASE("hasTeam matches TeamComponent", "[tower][team]")
{
	hl::Entity creep(1);
	creep.AddComponent<tower::TeamComponent>()->team = tower::Team::Creep;
	CHECK(tower::hasTeam(creep, tower::Team::Creep));
	CHECK_FALSE(tower::hasTeam(creep, tower::Team::Neutral));
	CHECK_FALSE(tower::hasTeam(static_cast<hl::Entity*>(nullptr), tower::Team::Creep));
}

TEST_CASE("towers auto-target Creep only", "[tower][team]")
{
	CHECK(tower::towerAutoTargets(tower::Team::Creep));
	CHECK_FALSE(tower::towerAutoTargets(tower::Team::Neutral));
	CHECK_FALSE(tower::towerAutoTargets(tower::Team::Tower));
}

TEST_CASE("nearestInRangeWithTeam ignores Neutral", "[tower][team][fire]")
{
	hl::Scene scene;
	auto* creep = scene.addEntity();
	creep->AddComponent<tower::TeamComponent>()->team = tower::Team::Creep;
	creep->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(1.0f, 0.0f, 0.0f));

	auto* rock = scene.addEntity();
	rock->AddComponent<tower::TeamComponent>()->team = tower::Team::Neutral;
	rock->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.5f, 0.0f, 0.0f));

	const glm::vec3 from{ 0.0f, 0.0f, 0.0f };
	CHECK(tower::nearestInRangeWithTeam(scene, from, 3.0f, tower::Team::Creep) == creep);
	CHECK(tower::nearestInRangeWithTeam(scene, from, 3.0f, tower::Team::Neutral) == rock);
	CHECK(tower::nearestInRangeWithTeam(scene, from, 0.25f, tower::Team::Creep) == nullptr);
}
