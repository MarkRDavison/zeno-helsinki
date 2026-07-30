#include <Systems/WeaponFiringSystem.hpp>
#include <Events/ShootEvent.hpp>
#include <helsinki/Engine/ECS/Components/SpriteComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/KinematicComponent.hpp>
#include <Components/EntityComponent.hpp>
#include <iostream>

namespace hur
{

	WeaponFiringSystem::WeaponFiringSystem(
		hl::EventBus& eventBus,
		hl::Scene& scene
	) :
		_eventBus(eventBus),
		_scene(scene)
	{
		_eventBus.AddListener(this);
	}
	WeaponFiringSystem::~WeaponFiringSystem()
	{
		_eventBus.RemoveListener(this);
	}

	void WeaponFiringSystem::update(float delta)
	{

	}

	void WeaponFiringSystem::OnEvent(const hl::Event& event)
	{
		if (auto se = dynamic_cast<const ShootEvent*>(&event))
		{
			const std::string texture = "laserBlue01";

			auto shooter = _scene.getEntity(se->getShooterId());
			const auto shooterPosition = shooter->GetComponent<hl::TransformComponent>()->GetPosition();

			auto projectile = _scene.addEntity();
			projectile->AddTag("PROJECTILE");
			projectile->AddTag("SPRITE"); // TODO: Maybe components can have tags that they auto add?
			projectile->AddComponent<hl::TransformComponent>()->SetPosition(shooterPosition - glm::vec3(0.0f, 64.0f, 0.0f));
			projectile->AddComponent<hl::KinematicComponent>()->velocity = { 0.0f, -384.0f, 0.0f };
			projectile->AddComponent<hl::SpriteComponent>();
			auto sc = projectile->AddComponent<EntityComponent>();
			sc->SpriteName = texture;
			sc->Size = { 9,54 };// TODO: LOOKUP SERVICE _spriteToIndexAndSize[sc->SpriteName].second;
		}
	}

}