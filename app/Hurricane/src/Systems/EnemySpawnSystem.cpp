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
		cc->mask = CollisionLayer::PlayerBullet;
		auto sc = enemy->AddComponent<EntityComponent>();
		sc->SpriteName = "enemyBlack1";
		sc->Size = _resourceService.getSize(sc->SpriteName);
		enemy->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(
			HurricaneConstants::Width / 2.0f,
			sc->Size.y / 2.0f + 16.0f,
			0.0f));
	}
}