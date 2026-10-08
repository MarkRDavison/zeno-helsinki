#include <Spawn.hpp>
#include <Components/CreepComponent.hpp>
#include <Components/EntityComponent.hpp>
#include <Components/HealthComponent.hpp>
#include <Components/PathFollowComponent.hpp>
#include <Components/StatusListComponent.hpp>
#include <Components/TeamComponent.hpp>
#include <Components/TowerComponent.hpp>
#include <SceneCatalog.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <Services/LevelCatalog.hpp>
#include <Services/TowerCatalog.hpp>
#include <helsinki/Engine/ECS/Components/ModelComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Renderer/Resource/ModelResource.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <stdexcept>
#include <string>

namespace
{
	void attachModel(hl::Entity* entity, hl::ResourceManager* resources, const std::string& modelId)
	{
		if (resources == nullptr)
		{
			return;
		}

		if (auto* model = resources->GetResource<hl::ModelResource>(modelId))
		{
			entity->AddComponent<hl::ModelComponent>()->setModelId(model->GetId());
		}
	}
}

namespace tower
{
	hl::Entity* spawnTower(
		hl::Scene& scene,
		const TowerDef& def,
		const LevelCatalog& level,
		int x,
		int z,
		hl::ResourceManager* resources)
	{
		auto* entity = scene.addEntity();
		entity->AddTag(TowerTag);
		entity->AddComponent<TeamComponent>()->team = Team::Tower;
		auto* tower = entity->AddComponent<TowerComponent>();
		tower->x = x;
		tower->z = z;
		tower->defId = def.id;
		tower->slotCooldown.assign(def.weapons.size(), 0.0f);
		entity->AddComponent<hl::TransformComponent>()->SetPosition(level.tileCenter(x, z));
		attachModel(entity, resources, def.model);
		return entity;
	}

	hl::Entity* spawnCreep(
		hl::Scene& scene,
		const CreepDef& def,
		const LevelCatalog& level,
		std::string_view pathName,
		hl::ResourceManager* resources)
	{
		const auto& start = level.path(pathName).front();
		auto* entity = scene.addEntity();
		entity->AddTag(CreepTag);
		entity->AddComponent<TeamComponent>()->team = Team::Creep;
		auto* transform = entity->AddComponent<hl::TransformComponent>();
		transform->SetPosition(level.tileCenter(start.x, start.z));
		transform->SetScale(glm::vec3(def.scale));
		attachModel(entity, resources, def.model);
		auto* follow = entity->AddComponent<PathFollowComponent>();
		follow->pathName = std::string(pathName);
		follow->fromIndex = 0;
		follow->t = 0.0f;
		follow->baseSpeed = def.speed;
		follow->speed = def.speed;
		auto* creep = entity->AddComponent<CreepComponent>();
		creep->resist = def.resist;
		creep->baseHealth = def.health;
		creep->baseSpeed = def.speed;
		creep->scale = def.scale;
		creep->range = def.range;
		creep->slots = def.slots;
		creep->slotCooldown.assign(def.slots.size(), 0.0f);
		entity->AddComponent<StatusListComponent>();
		auto* health = entity->AddComponent<HealthComponent>();
		health->max = def.health;
		health->current = def.health;
		return entity;
	}

	void spawnEntities(
		hl::Scene& scene,
		const LevelCatalog& level,
		const EntityCatalog& entities,
		hl::ResourceManager* resources)
	{
		for (const auto& placement : level.entities())
		{
			const auto* def = entities.find(placement.id);
			if (def == nullptr)
			{
				throw std::runtime_error(
					std::string("Unknown entity id '") + placement.id + "'");
			}

			auto* entity = scene.addEntity();
			entity->AddComponent<TeamComponent>()->team = Team::Neutral;
			auto* placed = entity->AddComponent<EntityComponent>();
			placed->x = placement.x;
			placed->z = placement.z;
			placed->sizeX = def->sizeX;
			placed->sizeZ = def->sizeZ;
			placed->resist = def->resist;
			if (def->health > 0.0f)
			{
				auto* health = entity->AddComponent<HealthComponent>();
				health->max = def->health;
				health->current = def->health;
			}
			const auto minCorner = level.tileCenter(placement.x, placement.z);
			const auto maxCorner = level.tileCenter(
				placement.x + def->sizeX - 1,
				placement.z + def->sizeZ - 1);
			auto* transform = entity->AddComponent<hl::TransformComponent>();
			transform->SetPosition((minCorner + maxCorner) * 0.5f);
			transform->SetScale(glm::vec3(
				static_cast<float>(def->sizeX),
				1.0f,
				static_cast<float>(def->sizeZ)));
			attachModel(entity, resources, def->model);
		}
	}
}
