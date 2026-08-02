#include <Systems/EnemySpawnSystem.hpp>
#include <Events/EnemySpawnEvent.hpp>
#include <Components/EntityComponent.hpp>
#include <Components/CollisionComponent.hpp>
#include <Components/HealthComponent.hpp>
#include <helsinki/Engine/ECS/Components/SpriteComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/KinematicComponent.hpp>
#include <HurricaneConstants.hpp>

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
				spawnDefaultEnemy();
			}
		}
	}

	void EnemySpawnSystem::OnEvent(const hl::Event& event)
	{
		if (auto ede = dynamic_cast<const EnemySpawnEvent*>(&event))
		{
			spawnDefaultEnemy();
		}
	}

	void EnemySpawnSystem::spawnDefaultEnemy()
	{
		auto enemy = _scene.addEntity();
		enemy->AddTag("SPRITE");
		enemy->AddTag("ENTITY");
		enemy->AddTag("COLLIDER");
		enemy->AddTag("ENEMY");
		enemy->AddComponent<hl::SpriteComponent>();
		enemy->AddComponent< HealthComponent>(10, 10);
		enemy->AddComponent<hl::KinematicComponent>()->velocity = glm::vec3(0.0f, 128.0f, 0.0f);
		auto cc = enemy->AddComponent<CollisionComponent>();
		cc->layer = CollisionLayer::Enemy;
		cc->mask = CollisionLayer::PlayerBullet | CollisionLayer::Player;
		auto sc = enemy->AddComponent<EntityComponent>();
		sc->SpriteName = "enemyBlack1";
		sc->Size = _resourceService.getSize(sc->SpriteName);

		const float x = sc->Size.x / 2.0f + static_cast<float>(rand() % 1000) / 1000.0f * (HurricaneConstants::Width - sc->Size.x);

		enemy->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(
			x,
			sc->Size.y / 2.0f + 16.0f,
			0.0f));
	}
}