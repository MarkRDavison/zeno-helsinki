#include <Systems/WeaponFiringSystem.hpp>
#include <Events/ShootEvent.hpp>
#include <helsinki/Engine/ECS/Components/SpriteComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/KinematicComponent.hpp>
#include <helsinki/Engine/ECS/Entity.hpp>
#include <Components/EntityComponent.hpp>
#include <Components/CollisionComponent.hpp>
#include <Components/ProjectileComponent.hpp>
#include <algorithm>

namespace hur
{

	WeaponFiringSystem::WeaponFiringSystem(
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
	WeaponFiringSystem::~WeaponFiringSystem()
	{
		_eventBus.RemoveListener(this);
	}

	void WeaponFiringSystem::update(float delta)
	{
		for (auto* enemy : _scene.getEntitiesByTag("ENEMY"))
		{
			if (enemy == nullptr || _scene.isPendingRemoval(enemy->Id))
			{
				continue;
			}

			auto* weapon = enemy->GetComponent<WeaponComponent>();
			if (weapon == nullptr || weapon->barrelCount <= 0)
			{
				continue;
			}

			weapon->fireCooldownRemaining = std::max(
				0.0f,
				weapon->fireCooldownRemaining - delta);

			if (weapon->fireCooldownRemaining <= 0.0f)
			{
				_eventBus.PublishEvent(ShootEvent(enemy->Id));
				weapon->fireCooldownRemaining = weapon->secondsPerShot;
			}
		}
	}

	void WeaponFiringSystem::spawnProjectile(
		const glm::vec3& position,
		const WeaponComponent& weapon,
		const hl::Entity& shooter)
	{
		auto* projectile = _scene.addEntity();
		projectile->AddTag("PROJECTILE");
		projectile->AddTag("COLLIDER");
		projectile->AddTag("SPRITE");
		auto* transform = projectile->AddComponent<hl::TransformComponent>();
		transform->SetPosition(position);
		if (weapon.projectileVelocity.y > 0.0f)
		{
			transform->SetScale({ 1.0f, -1.0f, 1.0f });
		}
		projectile->AddComponent<hl::KinematicComponent>()->velocity = weapon.projectileVelocity;
		projectile->AddComponent<hl::SpriteComponent>()->setFrameDataIndex(
			static_cast<int>(_resourceService.getIndex(weapon.projectileSprite)));
		auto* cc = projectile->AddComponent<CollisionComponent>();
		if (shooter.HasTag("ENEMY"))
		{
			cc->layer = CollisionLayer::EnemyBullet;
			cc->mask = CollisionLayer::Player;
		}
		else
		{
			cc->layer = CollisionLayer::PlayerBullet;
			cc->mask = CollisionLayer::Enemy;
		}
		projectile->AddComponent<ProjectileComponent>()->damage = weapon.damage;
		auto* sc = projectile->AddComponent<EntityComponent>();
		sc->SpriteName = weapon.projectileSprite;
		sc->Size = _resourceService.getSize(sc->SpriteName);
	}

	void WeaponFiringSystem::OnEvent(const hl::Event& event)
	{
		if (auto se = dynamic_cast<const ShootEvent*>(&event))
		{
			auto* shooter = _scene.getEntity(se->getShooterId());
			if (shooter == nullptr)
			{
				return;
			}

			auto* transform = shooter->GetComponent<hl::TransformComponent>();
			auto* weapon = shooter->GetComponent<WeaponComponent>();
			if (transform == nullptr || weapon == nullptr || weapon->barrelCount <= 0)
			{
				return;
			}

			const glm::vec3 shooterPosition = transform->GetPosition();
			const float muzzleSign = glm::sign(weapon->projectileVelocity.y);
			const float barrelCount = static_cast<float>(weapon->barrelCount);
			for (int i = 0; i < weapon->barrelCount; ++i)
			{
				const float xOffset =
					(static_cast<float>(i) - (barrelCount - 1.0f) * 0.5f) * weapon->spacing;
				spawnProjectile(
					glm::vec3(
						shooterPosition.x + xOffset,
						shooterPosition.y + muzzleSign * weapon->muzzleOffsetY,
						shooterPosition.z),
					*weapon,
					*shooter);
			}
		}
	}

}
