#include <Systems/ProjectileUpdateSystem.hpp>
#include <Components/EntityComponent.hpp>
#include <HurricaneConstants.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/KinematicComponent.hpp>

namespace hur
{
	ProjectileUpdateSystem::ProjectileUpdateSystem(
		hl::EventBus& eventBus,
		hl::Scene& scene
	) :
		_eventBus(eventBus),
		_scene(scene)
	{

	}

	void ProjectileUpdateSystem::update(float delta)
	{
		for (auto e : _scene.getEntitiesByTag("PROJECTILE"))
		{
			auto tc = e->GetComponent<hl::TransformComponent>();
			auto kc = e->GetComponent<hl::KinematicComponent>();
			auto ec = e->GetComponent<EntityComponent>();

			const auto newPosition = tc->GetPosition() + kc->velocity * delta;

			tc->SetPosition(newPosition);

			if (newPosition.y < -ec->Size.y)
			{
				_scene.removeEntity(e->Id);
			}
		}
	}
}