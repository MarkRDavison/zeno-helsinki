#include <Systems/TowerFireSystem.hpp>
#include <Components/TowerComponent.hpp>
#include <Services/LevelCatalog.hpp>
#include <Services/MatchContext.hpp>
#include <Services/WeaponCatalog.hpp>
#include <Services/ProjectileCatalog.hpp>
#include <SceneCatalog.hpp>
#include <Targeting.hpp>
#include <Fire.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
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

			tickSlotCooldowns(tower->slotCooldown, def->weapons.size(), delta);

			const glm::vec3 tile = _level.tileCenter(tower->x, tower->z);
			auto* creep = nearestInRangeWithTeam(_scene, tile, def->range, Team::Creep);
			if (creep == nullptr)
			{
				clampReadyCooldowns(tower->slotCooldown);
				continue;
			}

			const glm::vec3 creepPos = creep->GetComponent<hl::TransformComponent>()->GetPosition();
			const float yaw = glm::degrees(std::atan2(creepPos.x - tile.x, creepPos.z - tile.z));
			turnTowardYaw(
				*entity->GetComponent<hl::TransformComponent>(),
				yaw + TowerYawOffset,
				delta);
			const float aimedYaw = entity->GetComponent<hl::TransformComponent>()->GetRotation().y;

			fireSlots(
				_scene,
				_resourceManager,
				_weapons,
				_projectiles,
				tile,
				aimedYaw,
				*creep,
				def->weapons,
				tower->slotCooldown,
				[&](float cooldown) { return matchFireCooldown(cooldown, _match); });
		}
	}
}
