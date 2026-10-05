#include <Systems/TowerFireSystem.hpp>
#include <Components/TowerComponent.hpp>
#include <Components/ProjectileComponent.hpp>
#include <SceneCatalog.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/ModelComponent.hpp>
#include <helsinki/Renderer/Resource/ModelResource.hpp>

namespace tower
{
	namespace
	{
		float xzDistance(const glm::vec3& a, const glm::vec3& b)
		{
			const float dx = a.x - b.x;
			const float dz = a.z - b.z;
			return glm::length(glm::vec2(dx, dz));
		}

		hl::Entity* nearestCreepInRange(hl::Scene& scene, const glm::vec3& from)
		{
			hl::Entity* nearest = nullptr;
			float best = TowerRange;
			for (auto* creep : scene.getEntitiesWithComponents<hl::TransformComponent>(CreepTag))
			{
				if (scene.isPendingRemoval(creep->Id))
				{
					continue;
				}

				const float distance = xzDistance(
					from,
					creep->GetComponent<hl::TransformComponent>()->GetPosition());
				if (distance <= best)
				{
					best = distance;
					nearest = creep;
				}
			}

			return nearest;
		}
	}

	TowerFireSystem::TowerFireSystem(hl::Scene& scene, hl::ResourceManager& resourceManager) :
		_scene(scene),
		_resourceManager(resourceManager)
	{
	}

	void TowerFireSystem::update(float delta)
	{
		auto* model = _resourceManager.GetResource<hl::ModelResource>(ProjectileModelId);
		if (model == nullptr)
		{
			return;
		}

		for (auto* entity : _scene.getEntitiesWithComponents<hl::TransformComponent, TowerComponent>(TowerTag))
		{
			if (_scene.isPendingRemoval(entity->Id))
			{
				continue;
			}

			auto* tower = entity->GetComponent<TowerComponent>();
			tower->fireCooldownRemaining -= delta;
			if (tower->fireCooldownRemaining > 0.0f)
			{
				continue;
			}

			const glm::vec3 from = tileCenter(tower->x, tower->z, ProjectileY);
			auto* creep = nearestCreepInRange(_scene, from);
			if (creep == nullptr)
			{
				tower->fireCooldownRemaining = 0.0f;
				continue;
			}

			auto* projectile = _scene.addEntity();
			projectile->AddTag(ProjectileTag);
			auto* transform = projectile->AddComponent<hl::TransformComponent>();
			transform->SetPosition(from);
			transform->SetScale(ProjectileScale);
			projectile->AddComponent<hl::ModelComponent>()->setModelId(model->GetId());
			auto* shot = projectile->AddComponent<ProjectileComponent>();
			shot->targetId = creep->Id;
			shot->speed = ProjectileSpeed;
			shot->damage = ProjectileDamage;
			const glm::vec3 creepPos = creep->GetComponent<hl::TransformComponent>()->GetPosition();
			shot->lastDest = glm::vec3(creepPos.x, ProjectileY, creepPos.z);
			tower->fireCooldownRemaining = TowerFireCooldown;
		}
	}
}
