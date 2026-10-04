#include <Systems/EntityDeathSystem.hpp>
#include <Events/EntityDeathEvent.hpp>
#include <Events/PlayerLifeLostEvent.hpp>
#include <Events/PlayerScoreEvent.hpp>
#include <Components/CollisionComponent.hpp>
#include <Components/EntityComponent.hpp>
#include <Components/PickupComponent.hpp>
#include <Components/SpriteClipComponent.hpp>
#include <EntityCatalog.hpp>
#include <HurricaneConstants.hpp>
#include <helsinki/Engine/ECS/Components/SpriteComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/KinematicComponent.hpp>

namespace hur
{
	EntityDeathSystem::EntityDeathSystem(
		hl::EventBus& eventBus,
		hl::Scene& scene,
		const ResourceService& resourceService
	) :
		_eventBus(eventBus),
		_scene(scene),
		_resourceService(resourceService)
	{
		_eventBus.AddListener(this);
	}

	EntityDeathSystem::~EntityDeathSystem()
	{
		_eventBus.RemoveListener(this);
	}

	void EntityDeathSystem::update(float delta)
	{

	}

	void EntityDeathSystem::spawnSpriteClip(
		const glm::vec3& position,
		std::span<const char* const> names,
		float secondsPerFrame)
	{
		if (names.empty())
		{
			return;
		}

		auto* entity = _scene.addEntity();
		entity->AddTag("SPRITE");

		auto* clip = entity->AddComponent<SpriteClipComponent>();
		clip->frames.reserve(names.size());
		clip->secondsPerFrame = secondsPerFrame;
		clip->destroyOnComplete = true;

		for (const char* name : names)
		{
			clip->frames.push_back(SpriteClipFrame{
				.frameIndex = static_cast<int>(_resourceService.getIndex(name)),
				.size = _resourceService.getSize(name),
			});
		}

		auto* ec = entity->AddComponent<EntityComponent>();
		ec->SpriteName = names.front();
		ec->Size = clip->frames.front().size;

		entity->AddComponent<hl::SpriteComponent>()->setFrameDataIndex(clip->frames.front().frameIndex);
		entity->AddComponent<hl::TransformComponent>()->SetPosition(position);
	}

	void EntityDeathSystem::spawnPickup(const glm::vec3& position, const char* pickupId)
	{
		auto* entity = _scene.addEntity();
		entity->AddTag("SPRITE");
		entity->AddTag("ENTITY");
		entity->AddTag("COLLIDER");
		entity->AddTag("PICKUP");

		auto* pickup = entity->AddComponent<PickupComponent>();
		pickup->pickupId = pickupId;

		auto* ec = entity->AddComponent<EntityComponent>();
		ec->SpriteName = pickupId;
		ec->Size = _resourceService.getSize(pickupId);

		auto* cc = entity->AddComponent<CollisionComponent>();
		cc->layer = CollisionLayer::Pickup;
		cc->mask = CollisionLayer::Player;

		entity->AddComponent<hl::SpriteComponent>()->setFrameDataIndex(
			static_cast<int>(_resourceService.getIndex(pickupId)));
		entity->AddComponent<hl::TransformComponent>()->SetPosition(position);
		entity->AddComponent<hl::KinematicComponent>()->velocity = glm::vec3(0.0f, PickupFallSpeedY, 0.0f);
	}

	void EntityDeathSystem::OnEvent(const hl::Event& event)
	{
		if (auto ede = dynamic_cast<const EntityDeathEvent*>(&event))
		{
			auto entity = _scene.getEntity(ede->getId());
			if (entity == nullptr)
			{
				return;
			}

			if (entity->HasTag("PLAYER")) // TODO: CONSTANT
			{
				_eventBus.PublishEvent(PlayerLifeLostEvent());
			}
			else
			{
				if (auto* transform = entity->GetComponent<hl::TransformComponent>())
				{
					spawnSpriteClip(
						transform->GetPosition(),
						DeathVfxSpriteNames,
						DeathVfxSecondsPerFrame);

					if (ede->getDeathType() == DeathType::DROP)
					{
						if (auto* ec = entity->GetComponent<EntityComponent>())
						{
							if (const EntityDefinition* def = findEntityDefinition(ec->Type))
							{
								if (const char* pickupId = rollLoot(*def))
								{
									spawnPickup(transform->GetPosition(), pickupId);
								}
							}
						}
					}
				}

				_eventBus.PublishEvent(PlayerScoreEvent(1));
			}

			_scene.removeEntity(entity->Id);
		}
	}
}
