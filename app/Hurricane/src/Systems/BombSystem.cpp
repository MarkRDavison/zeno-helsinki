#include <Systems/BombSystem.hpp>
#include <Events/BombEvent.hpp>
#include <Events/EntityDeathEvent.hpp>
#include <Events/MissileExplodeEvent.hpp>
#include <Components/CollisionComponent.hpp>
#include <Components/EntityComponent.hpp>
#include <Components/SpriteClipComponent.hpp>
#include <HurricaneConstants.hpp>
#include <helsinki/Engine/ECS/Components/KinematicComponent.hpp>
#include <helsinki/Engine/ECS/Components/SpriteComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <algorithm>
#include <vector>

namespace hur
{
	BombSystem::BombSystem(
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

	BombSystem::~BombSystem()
	{
		_eventBus.RemoveListener(this);
	}

	void BombSystem::update(float delta)
	{
	}

	void BombSystem::spawnSpriteClip(
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

	void BombSystem::spawnMissile(const glm::vec3& playerPosition)
	{
		auto* missile = _scene.addEntity();
		missile->AddTag("PROJECTILE");
		missile->AddTag("MISSILE");
		missile->AddTag("COLLIDER");
		missile->AddTag("SPRITE");

		const glm::vec3 position{
			playerPosition.x,
			playerPosition.y - BombMissileMuzzleOffsetY,
			playerPosition.z };

		missile->AddComponent<hl::TransformComponent>()->SetPosition(position);
		missile->AddComponent<hl::KinematicComponent>()->velocity = BombMissileVelocity;
		missile->AddComponent<hl::SpriteComponent>()->setFrameDataIndex(
			static_cast<int>(_resourceService.getIndex(BombMissileSprite)));

		auto* cc = missile->AddComponent<CollisionComponent>();
		cc->layer = CollisionLayer::PlayerBullet;
		cc->mask = CollisionLayer::Enemy;

		auto* ec = missile->AddComponent<EntityComponent>();
		ec->Type = "player_missile";
		ec->SpriteName = BombMissileSprite;
		ec->Size = _resourceService.getSize(ec->SpriteName);
	}

	void BombSystem::detonateAt(const glm::vec3& position)
	{
		spawnSpriteClip(position, BombBlastSpriteNames, BombBlastSecondsPerFrame);

		const glm::vec2 blastCentre{ position.x, position.y };
		std::vector<int> killIds;

		for (auto* enemy : _scene.getEntitiesByTag("ENEMY"))
		{
			if (enemy == nullptr || _scene.isPendingRemoval(enemy->Id))
			{
				continue;
			}

			auto* enemyTransform = enemy->GetComponent<hl::TransformComponent>();
			auto* enemyEntity = enemy->GetComponent<EntityComponent>();
			if (enemyTransform == nullptr || enemyEntity == nullptr)
			{
				continue;
			}

			const glm::vec3 enemyPosition = enemyTransform->GetPosition();
			const glm::vec2 enemyCentre{ enemyPosition.x, enemyPosition.y };
			const float halfSize = 0.5f * std::max(enemyEntity->Size.x, enemyEntity->Size.y);
			if (glm::distance(blastCentre, enemyCentre) <= BombAoeRadius + halfSize)
			{
				killIds.push_back(enemy->Id);
			}
		}

		for (int id : killIds)
		{
			if (_scene.isPendingRemoval(id))
			{
				continue;
			}

			_eventBus.PublishEvent(EntityDeathEvent(id, DeathType::DROP));
		}
	}

	void BombSystem::OnEvent(const hl::Event& event)
	{
		if (auto* bombEvent = dynamic_cast<const BombEvent*>(&event))
		{
			auto* player = _scene.getEntity(bombEvent->getPlayerId());
			if (player == nullptr || _scene.isPendingRemoval(player->Id))
			{
				return;
			}

			auto* transform = player->GetComponent<hl::TransformComponent>();
			if (transform == nullptr)
			{
				return;
			}

			spawnMissile(transform->GetPosition());
			return;
		}

		if (auto* explodeEvent = dynamic_cast<const MissileExplodeEvent*>(&event))
		{
			detonateAt(explodeEvent->getPosition());
		}
	}
}
