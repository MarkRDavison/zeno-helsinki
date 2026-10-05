#include <Systems/ProjectileSystem.hpp>
#include <Components/ProjectileComponent.hpp>
#include <SceneCatalog.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>

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
	}

	ProjectileSystem::ProjectileSystem(hl::Scene& scene) :
		_scene(scene)
	{
	}

	void ProjectileSystem::update(float delta)
	{
		for (auto* entity : _scene.getEntitiesWithComponents<hl::TransformComponent, ProjectileComponent>(ProjectileTag))
		{
			if (_scene.isPendingRemoval(entity->Id))
			{
				continue;
			}

			auto* shot = entity->GetComponent<ProjectileComponent>();
			auto* target = _scene.getEntity(shot->targetId);
			if (target == nullptr
				|| _scene.isPendingRemoval(target->Id)
				|| !target->HasComponent<hl::TransformComponent>())
			{
				_scene.removeEntity(entity->Id);
				continue;
			}

			auto* transform = entity->GetComponent<hl::TransformComponent>();
			const glm::vec3 pos = transform->GetPosition();
			const glm::vec3 targetPos = target->GetComponent<hl::TransformComponent>()->GetPosition();
			const glm::vec3 dest{ targetPos.x, ProjectileY, targetPos.z };
			const float distance = xzDistance(pos, dest);
			const float step = shot->speed * delta;
			if (distance <= ProjectileHitRadius || distance <= step)
			{
				_scene.removeEntity(target->Id);
				_scene.removeEntity(entity->Id);
				continue;
			}

			const glm::vec3 dir = glm::normalize(glm::vec3(dest.x - pos.x, 0.0f, dest.z - pos.z));
			transform->SetPosition(glm::vec3(
				pos.x + dir.x * step,
				ProjectileY,
				pos.z + dir.z * step));
		}
	}
}
