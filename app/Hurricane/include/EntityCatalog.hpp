#pragma once

#include <WeaponCatalog.hpp>
#include <cstdlib>
#include <span>
#include <string_view>

namespace hur
{
	inline constexpr const char* EntityTypeBasicEnemy = "basic_enemy";
	inline constexpr const char* EntityTypeToughEnemy = "tough_enemy";
	inline constexpr const char* EntityTypeRewardEnemy = "reward_enemy";

	struct WeightedPickup
	{
		const char* pickupId;
		int weight;
	};

	struct WeightedSpawn
	{
		const char* typeId;
		int weight;
	};

	struct EntityDefinition
	{
		const char* id;
		const char* sprite;
		int health;
		float speedY;
		std::span<const WeightedPickup> loot;
		int missWeight;
		const char* weaponId;
	};

	inline constexpr WeightedPickup BasicEnemyLoot[] = {
		{ "powerupBlue_bolt", 1 },
		{ "powerupBlue_star", 1 },
		{ PickupIdSpeed, 1 },
		{ PickupIdShield, 1 },
	};

	inline constexpr WeightedPickup ToughEnemyLoot[] = {
		{ "powerupBlue_bolt", 2 },
		{ "powerupBlue_star", 1 },
		{ PickupIdSpeed, 1 },
		{ PickupIdShield, 1 },
	};

	inline constexpr WeightedPickup RewardEnemyLoot[] = {
		{ "powerupBlue_bolt", 3 },
		{ "powerupBlue_star", 1 },
		{ PickupIdSpeed, 1 },
		{ PickupIdShield, 1 },
	};

	inline constexpr WeightedSpawn kEnemySpawnWeights[] = {
		{ EntityTypeBasicEnemy, 6 },
		{ EntityTypeRewardEnemy, 2 },
		{ EntityTypeToughEnemy, 1 },
	};

	inline const EntityDefinition kEntityDefinitions[] = {
		{
			.id = EntityTypeBasicEnemy,
			.sprite = "enemyGreen1",
			.health = 10,
			.speedY = 128.0f,
			.loot = std::span<const WeightedPickup>{ BasicEnemyLoot },
			.missWeight = 2,
			.weaponId = WeaponTypeEnemyLaser,
		},
		{
			.id = EntityTypeToughEnemy,
			.sprite = "enemyBlack1",
			.health = 20,
			.speedY = 96.0f,
			.loot = std::span<const WeightedPickup>{ ToughEnemyLoot },
			.missWeight = 2,
			.weaponId = WeaponTypeEnemyHeavyMissile,
		},
		{
			.id = EntityTypeRewardEnemy,
			.sprite = "enemyRed1",
			.health = 5,
			.speedY = 160.0f,
			.loot = std::span<const WeightedPickup>{ RewardEnemyLoot },
			.missWeight = 1,
			.weaponId = nullptr,
		},
	};

	inline const EntityDefinition* findEntityDefinition(std::string_view type)
	{
		if (type.empty())
		{
			return nullptr;
		}

		for (const auto& def : kEntityDefinitions)
		{
			if (type == def.id)
			{
				return &def;
			}
		}

		return nullptr;
	}

	inline const EntityDefinition* rollSpawnDefinition()
	{
		int total = 0;
		for (const auto& entry : kEnemySpawnWeights)
		{
			total += entry.weight;
		}

		if (total <= 0)
		{
			return nullptr;
		}

		int roll = std::rand() % total;
		for (const auto& entry : kEnemySpawnWeights)
		{
			if (roll < entry.weight)
			{
				return findEntityDefinition(entry.typeId);
			}

			roll -= entry.weight;
		}

		return nullptr;
	}

	inline const char* rollLoot(const EntityDefinition& def)
	{
		int total = def.missWeight;
		for (const auto& entry : def.loot)
		{
			total += entry.weight;
		}

		if (total <= 0)
		{
			return nullptr;
		}

		int roll = std::rand() % total;
		if (roll < def.missWeight)
		{
			return nullptr;
		}

		roll -= def.missWeight;
		for (const auto& entry : def.loot)
		{
			if (roll < entry.weight)
			{
				return entry.pickupId;
			}

			roll -= entry.weight;
		}

		return nullptr;
	}
}
