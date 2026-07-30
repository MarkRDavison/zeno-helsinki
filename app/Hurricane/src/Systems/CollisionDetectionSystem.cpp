#include <Systems/CollisionDetectionSystem.hpp>
#include <HurricaneConstants.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <Components/EntityComponent.hpp>
#include <Components/CollisionComponent.hpp>
#include <Events/CollisionEvent.hpp>

namespace hur
{
	CollisionDetectionSystem::CollisionDetectionSystem(
		hl::EventBus& eventBus,
		hl::Scene& scene
	) :
		_eventBus(eventBus),
		_scene(scene)
	{

	}

	void CollisionDetectionSystem::update(float delta)
	{
		const auto colliders = _scene.getEntitiesByTag("COLLIDER");

		for (auto i = 0; i < colliders.size(); ++i)
		{
			auto colliderEntityI = colliders[i];

			const auto iTc = colliderEntityI->GetComponent<hl::TransformComponent>();
			const auto iEc = colliderEntityI->GetComponent<EntityComponent>();
			const auto iCc = colliderEntityI->GetComponent<CollisionComponent>();

			const auto iPos = iTc->GetPosition();
			const auto iSize = iEc->Size;

			const auto iLeft = iPos.x - iSize.x / 2.0f;
			const auto iRight = iPos.x + iSize.x / 2.0f;

			const auto iTop = iPos.y - iSize.y / 2.0f;
			const auto iBottom = iPos.y + iSize.y / 2.0f;

			for (auto j = i + 1; j < colliders.size(); ++j)
			{
				auto colliderEntityJ = colliders[j];

				const auto jTc = colliderEntityJ->GetComponent<hl::TransformComponent>();
				const auto jEc = colliderEntityJ->GetComponent<EntityComponent>();
				const auto jCc = colliderEntityJ->GetComponent<CollisionComponent>();

				if (!ShouldTest(*iCc, *jCc))
				{
					continue;
				}

				const auto jPos = jTc->GetPosition();
				const auto jSize = jEc->Size;

				const auto jLeft = jPos.x - jSize.x / 2.0f;
				const auto jRight = jPos.x + jSize.x / 2.0f;

				const auto jTop = jPos.y - jSize.y / 2.0f;
				const auto jBottom = jPos.y + jSize.y / 2.0f;

				if (iLeft > jRight ||
					iRight < jLeft ||
					iTop > jBottom ||
					iBottom < jTop)
				{
					continue;
				}

				_eventBus.PublishEvent(CollisionEvent(colliderEntityI->Id, colliderEntityJ->Id));
			}
		}
	}
}