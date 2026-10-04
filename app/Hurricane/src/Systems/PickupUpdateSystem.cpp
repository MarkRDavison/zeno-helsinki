#include <Systems/PickupUpdateSystem.hpp>
#include <Components/EntityComponent.hpp>
#include <HurricaneConstants.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/KinematicComponent.hpp>

namespace hur
{
	PickupUpdateSystem::PickupUpdateSystem(
		hl::EventBus& eventBus,
		hl::Scene& scene
	) :
		_eventBus(eventBus),
		_scene(scene)
	{
	}

	void PickupUpdateSystem::update(float delta)
	{
		for (auto* e : _scene.getEntitiesByTag("PICKUP"))
		{
			if (_scene.isPendingRemoval(e->Id))
			{
				continue;
			}

			auto* tc = e->GetComponent<hl::TransformComponent>();
			auto* kc = e->GetComponent<hl::KinematicComponent>();
			auto* ec = e->GetComponent<EntityComponent>();

			const auto newPosition = tc->GetPosition() + kc->velocity * delta;
			tc->SetPosition(newPosition);

			if (newPosition.y + ec->Size.y / 2.0f > HurricaneConstants::Height)
			{
				_scene.removeEntity(e->Id);
			}
		}
	}
}
