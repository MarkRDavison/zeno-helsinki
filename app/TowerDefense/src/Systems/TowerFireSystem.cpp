#include <Systems/TowerFireSystem.hpp>
#include <Components/TowerComponent.hpp>
#include <Components/ProjectileComponent.hpp>
#include <Services/LevelCatalog.hpp>
#include <Services/MatchContext.hpp>
#include <Services/WeaponCatalog.hpp>
#include <Services/ProjectileCatalog.hpp>
#include <SceneCatalog.hpp>
#include <Targeting.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/ModelComponent.hpp>
#include <helsinki/Renderer/Resource/ModelResource.hpp>
#include <cmath>

namespace tower
{
	namespace
	{
		float wrapDegrees(float degrees)
		{
			degrees = std::fmod(degrees + 180.0f, 360.0f);
			if (degrees < 0.0f)
			{
				degrees += 360.0f;
			}
			return degrees - 180.0f;
		}

		void turnTowardYaw(hl::TransformComponent& transform, float targetYaw, float delta)
		{
			const float current = transform.GetRotation().y;
			const float remaining = wrapDegrees(targetYaw - current);
			const float maxStep = TowerTurnSpeed * delta;
			const float next = std::abs(remaining) <= maxStep
				? targetYaw
				: current + std::copysign(maxStep, remaining);
			transform.SetRotation(glm::vec3(0.0f, next, 0.0f));
		}

		glm::vec3 rotateYaw(const glm::vec3& local, float yawDegrees)
		{
			const float rad = glm::radians(yawDegrees);
			const float c = std::cos(rad);
			const float s = std::sin(rad);
			return glm::vec3(
				local.x * c + local.z * s,
				local.y,
				-local.x * s + local.z * c);
		}
	}

	TowerFireSystem::TowerFireSystem(
		hl::Scene& scene,
		hl::ResourceManager& resourceManager,
		TowerCatalog& towers,
		WeaponCatalog& weapons,
		ProjectileCatalog& projectiles,
		LevelCatalog& level,
		MatchContext& match) :
		_scene(scene),
		_resourceManager(resourceManager),
		_towers(towers),
		_weapons(weapons),
		_projectiles(projectiles),
		_level(level),
		_match(match)
	{
	}

	void TowerFireSystem::update(float delta)
	{
		for (auto* entity : _scene.getEntitiesWithComponents<hl::TransformComponent, TowerComponent>(TowerTag))
		{
			if (_scene.isPendingRemoval(entity->Id))
			{
				continue;
			}

			auto* tower = entity->GetComponent<TowerComponent>();
			const auto* def = _towers.find(tower->defId);
			if (def == nullptr)
			{
				continue;
			}

			if (tower->slotCooldown.size() != def->weapons.size())
			{
				tower->slotCooldown.assign(def->weapons.size(), 0.0f);
			}

			for (float& remaining : tower->slotCooldown)
			{
				remaining -= delta;
			}

			const glm::vec3 tile = _level.tileCenter(tower->x, tower->z);
			auto* creep = nearestInRangeWithTeam(_scene, tile, def->range, Team::Creep);
			if (creep == nullptr)
			{
				for (float& remaining : tower->slotCooldown)
				{
					if (remaining < 0.0f)
					{
						remaining = 0.0f;
					}
				}
				continue;
			}

			const glm::vec3 creepPos = creep->GetComponent<hl::TransformComponent>()->GetPosition();
			const float yaw = glm::degrees(std::atan2(creepPos.x - tile.x, creepPos.z - tile.z));
			turnTowardYaw(
				*entity->GetComponent<hl::TransformComponent>(),
				yaw + TowerYawOffset,
				delta);
			const float aimedYaw = entity->GetComponent<hl::TransformComponent>()->GetRotation().y;

			for (std::size_t i = 0; i < def->weapons.size(); ++i)
			{
				if (tower->slotCooldown[i] > 0.0f)
				{
					continue;
				}

				const auto* weapon = _weapons.find(def->weapons[i].id);
				const auto* projectile = (weapon != nullptr)
					? _projectiles.find(weapon->projectile)
					: nullptr;
				if (weapon == nullptr || projectile == nullptr)
				{
					continue;
				}

				auto* model = _resourceManager.GetResource<hl::ModelResource>(projectile->model);
				const glm::vec3 worldOffset = rotateYaw(def->weapons[i].offset, aimedYaw);
				const glm::vec3 from = tile + worldOffset;
				const glm::vec3 lateral = rotateYaw(
					glm::vec3(def->weapons[i].offset.x, 0.0f, 0.0f),
					aimedYaw);
				const glm::vec3 dest = glm::vec3(
					creepPos.x + lateral.x,
					projectile->y,
					creepPos.z + lateral.z);

				auto* shotEntity = _scene.addEntity();
				shotEntity->AddTag(ProjectileTag);
				auto* transform = shotEntity->AddComponent<hl::TransformComponent>();
				transform->SetPosition(glm::vec3(from.x, projectile->y, from.z));
				transform->SetScale(projectile->scale);
				if (model != nullptr)
				{
					shotEntity->AddComponent<hl::ModelComponent>()->setModelId(model->GetId());
				}

				auto* shot = shotEntity->AddComponent<ProjectileComponent>();
				shot->targetId = creep->Id;
				shot->speed = projectile->speed;
				shot->damage = projectile->damage;
				shot->damageType = projectile->damageType;
				shot->statuses = projectile->statuses;
				shot->hitRadius = projectile->hitRadius;
				shot->y = projectile->y;
				shot->aimOffset = glm::vec2(lateral.x, lateral.z);
				shot->lastDest = dest;
				tower->slotCooldown[i] = matchFireCooldown(weapon->fireCooldown, _match);
			}
		}
	}
}
