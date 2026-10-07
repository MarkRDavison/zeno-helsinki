#pragma once

#include <Armed.hpp>
#include <Components/ProjectileComponent.hpp>
#include <SceneCatalog.hpp>
#include <Services/ProjectileCatalog.hpp>
#include <Services/WeaponCatalog.hpp>
#include <helsinki/Engine/ECS/Components/ModelComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Entity.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/Renderer/Resource/ModelResource.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <helsinki/System/glm.hpp>
#include <cmath>
#include <functional>
#include <vector>

namespace tower
{
	inline glm::vec3 fireRotateYaw(const glm::vec3& local, float yawDegrees)
	{
		const float rad = glm::radians(yawDegrees);
		const float c = std::cos(rad);
		const float s = std::sin(rad);
		return glm::vec3(
			local.x * c + local.z * s,
			local.y,
			-local.x * s + local.z * c);
	}

	inline void tickSlotCooldowns(std::vector<float>& remaining, std::size_t slotCount, float delta)
	{
		if (remaining.size() != slotCount)
		{
			remaining.assign(slotCount, 0.0f);
		}

		for (float& time : remaining)
		{
			time -= delta;
		}
	}

	inline void clampReadyCooldowns(std::vector<float>& remaining)
	{
		for (float& time : remaining)
		{
			if (time < 0.0f)
			{
				time = 0.0f;
			}
		}
	}

	inline void spawnShot(
		hl::Scene& scene,
		hl::ResourceManager& resourceManager,
		const ProjectileDef& projectile,
		const glm::vec3& from,
		const glm::vec3& targetPos,
		float aimedYaw,
		const glm::vec3& slotOffset,
		hl::Entity& target)
	{
		auto* model = resourceManager.GetResource<hl::ModelResource>(projectile.model);
		const glm::vec3 worldOffset = fireRotateYaw(slotOffset, aimedYaw);
		const glm::vec3 origin = from + worldOffset;
		const glm::vec3 lateral = fireRotateYaw(glm::vec3(slotOffset.x, 0.0f, 0.0f), aimedYaw);
		const glm::vec3 dest = glm::vec3(
			targetPos.x + lateral.x,
			projectile.y,
			targetPos.z + lateral.z);

		auto* shotEntity = scene.addEntity();
		shotEntity->AddTag(ProjectileTag);
		auto* transform = shotEntity->AddComponent<hl::TransformComponent>();
		transform->SetPosition(glm::vec3(origin.x, projectile.y, origin.z));
		transform->SetScale(projectile.scale);
		if (model != nullptr)
		{
			shotEntity->AddComponent<hl::ModelComponent>()->setModelId(model->GetId());
		}

		auto* shot = shotEntity->AddComponent<ProjectileComponent>();
		shot->targetId = target.Id;
		shot->speed = projectile.speed;
		shot->damage = projectile.damage;
		shot->damageType = projectile.damageType;
		shot->statuses = projectile.statuses;
		shot->hitRadius = projectile.hitRadius;
		shot->y = projectile.y;
		shot->aimOffset = glm::vec2(lateral.x, lateral.z);
		shot->lastDest = dest;
	}

	inline void fireSlots(
		hl::Scene& scene,
		hl::ResourceManager& resourceManager,
		const WeaponCatalog& weapons,
		const ProjectileCatalog& projectiles,
		const glm::vec3& from,
		float aimedYaw,
		hl::Entity& target,
		const std::vector<WeaponSlot>& slots,
		std::vector<float>& slotCooldown,
		const std::function<float(float)>& cooldownOf)
	{
		const glm::vec3 targetPos = target.GetComponent<hl::TransformComponent>()->GetPosition();
		for (std::size_t i = 0; i < slots.size(); ++i)
		{
			if (slotCooldown[i] > 0.0f)
			{
				continue;
			}

			const auto* weapon = weapons.find(slots[i].id);
			const auto* projectile = (weapon != nullptr)
				? projectiles.find(weapon->projectile)
				: nullptr;
			if (weapon == nullptr || projectile == nullptr)
			{
				continue;
			}

			spawnShot(
				scene,
				resourceManager,
				*projectile,
				from,
				targetPos,
				aimedYaw,
				slots[i].offset,
				target);
			slotCooldown[i] = cooldownOf(weapon->fireCooldown);
		}
	}
}
