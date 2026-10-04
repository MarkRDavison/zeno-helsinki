#include <Systems/EnemySpawnSystem.hpp>
#include <Events/EnemySpawnEvent.hpp>
#include <Components/EntityComponent.hpp>
#include <Components/CollisionComponent.hpp>
#include <Components/HealthComponent.hpp>
#include <helsinki/Engine/ECS/Components/SpriteComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/KinematicComponent.hpp>
#include <HurricaneConstants.hpp>
#include <EntityCatalog.hpp>

namespace hur
{

	EnemySpawnSystem::EnemySpawnSystem(
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
	EnemySpawnSystem::~EnemySpawnSystem()
	{
		_eventBus.RemoveListener(this);
	}

	void EnemySpawnSystem::update(float delta)
	{
		_elapsed += delta;

		if (_elapsed > 2.0f)
		{
			_elapsed -= 2.0f;

			const auto enemyCount = _scene.getEntitiesByTag("ENEMY").size();

			if (enemyCount < 3)
			{
				if (const EntityDefinition* def = rollSpawnDefinition())
				{
					spawnEnemy(*def);
				}
			}
		}
	}

	void EnemySpawnSystem::OnEvent(const hl::Event& event)
	{
		if (dynamic_cast<const EnemySpawnEvent*>(&event) != nullptr)
		{
			if (const EntityDefinition* def = rollSpawnDefinition())
			{
				spawnEnemy(*def);
			}
		}
	}

	void EnemySpawnSystem::spawnEnemy(const EntityDefinition& def)
	{
		auto enemy = _scene.addEntity();
		enemy->AddTag("SPRITE");
		enemy->AddTag("ENTITY");
		enemy->AddTag("COLLIDER");
		enemy->AddTag("ENEMY");
		auto sc = enemy->AddComponent<EntityComponent>();
		sc->Type = def.id;
		sc->SpriteName = def.sprite;
		sc->Size = _resourceService.getSize(sc->SpriteName);
		enemy->AddComponent<hl::SpriteComponent>()->setFrameDataIndex(
			static_cast<int>(_resourceService.getIndex(sc->SpriteName)));
		enemy->AddComponent< HealthComponent>(def.health, def.health);
		enemy->AddComponent<hl::KinematicComponent>()->velocity = glm::vec3(0.0f, def.speedY, 0.0f);
		auto cc = enemy->AddComponent<CollisionComponent>();
		cc->layer = CollisionLayer::Enemy;
		cc->mask = CollisionLayer::PlayerBullet | CollisionLayer::Player;

		if (def.weaponId != nullptr && def.weaponId[0] != '\0')
		{
			auto* weapon = enemy->AddComponent<WeaponComponent>();
			applyWeapon(*weapon, def.weaponId);
			if (weapon->secondsPerShot > 0.0f)
			{
				weapon->fireCooldownRemaining =
					static_cast<float>(rand() % 1000) / 1000.0f * weapon->secondsPerShot;
			}
		}

		const float x = sc->Size.x / 2.0f + static_cast<float>(rand() % 1000) / 1000.0f * (HurricaneConstants::Width - sc->Size.x);

		enemy->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(
			x,
			sc->Size.y / 2.0f + 16.0f,
			0.0f));
	}
}