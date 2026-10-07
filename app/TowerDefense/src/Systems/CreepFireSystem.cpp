#include <Systems/CreepFireSystem.hpp>
#include <Components/CreepComponent.hpp>
#include <Components/PathFollowComponent.hpp>
#include <Components/TeamComponent.hpp>
#include <Armed.hpp>
#include <Fire.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <cmath>

namespace tower
{
	CreepFireSystem::CreepFireSystem(
		hl::Scene& scene,
		hl::ResourceManager& resourceManager,
		WeaponCatalog& weapons,
		ProjectileCatalog& projectiles) :
		_scene(scene),
		_resourceManager(resourceManager),
		_weapons(weapons),
		_projectiles(projectiles)
	{
	}

	void CreepFireSystem::update(float delta)
	{
		for (auto* entity : _scene.getEntitiesWithComponents<
			hl::TransformComponent,
			PathFollowComponent,
			CreepComponent,
			TeamComponent>())
		{
			if (_scene.isPendingRemoval(entity->Id) || !hasTeam(entity, Team::Creep))
			{
				continue;
			}

			auto* creep = entity->GetComponent<CreepComponent>();
			tickSlotCooldowns(creep->slotCooldown, creep->slots.size(), delta);

			const auto* follow = entity->GetComponent<PathFollowComponent>();
			auto* target = follow->stalledEntityId >= 0
				? _scene.getEntity(follow->stalledEntityId)
				: nullptr;
			if (target != nullptr
				&& (_scene.isPendingRemoval(target->Id) || !hasTeam(target, Team::Neutral)))
			{
				target = nullptr;
			}

			if (!canFireAtStalled(creep->slots, target))
			{
				clampReadyCooldowns(creep->slotCooldown);
				continue;
			}

			auto* transform = entity->GetComponent<hl::TransformComponent>();
			const glm::vec3 from = transform->GetPosition();
			const glm::vec3 targetPos = target->GetComponent<hl::TransformComponent>()->GetPosition();
			const float yaw = glm::degrees(std::atan2(targetPos.x - from.x, targetPos.z - from.z));
			fireSlots(
				_scene,
				_resourceManager,
				_weapons,
				_projectiles,
				from,
				yaw,
				*target,
				creep->slots,
				creep->slotCooldown,
				[](float cooldown) { return cooldown; });
		}
	}
}
