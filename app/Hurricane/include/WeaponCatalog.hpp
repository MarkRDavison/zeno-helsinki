#pragma once

#include <HurricaneConstants.hpp>
#include <helsinki/Engine/ECS/Component.hpp>
#include <helsinki/System/glm.hpp>
#include <string>
#include <string_view>

namespace hur
{
	inline constexpr const char* WeaponTypeSingleLaser = "single_laser";
	inline constexpr const char* WeaponTypeDualLaser = "dual_laser";
	inline constexpr const char* WeaponTypeEnemyLaser = "enemy_laser";
	inline constexpr const char* WeaponTypeEnemyHeavyMissile = "enemy_heavy_missile";

	struct WeaponDefinition
	{
		const char* id;
		int barrelCount;
		float spacing;
		float muzzleOffsetY;
		float secondsPerShot;
		int damage;
		glm::vec3 projectileVelocity;
		const char* projectileSprite;
	};

	class WeaponComponent : public hl::Component
	{
	public:
		std::string Type;
		int barrelCount{ 1 };
		float spacing{ 0.0f };
		float muzzleOffsetY{ 64.0f };
		float secondsPerShot{ LaserSecondsPerShot };
		float fireCooldownRemaining{ 0.0f };
		int damage{ PlayerLaserDamage };
		glm::vec3 projectileVelocity{ 0.0f, -384.0f, 0.0f };
		std::string projectileSprite{ "laserBlue01" };
	};

	inline const WeaponDefinition kWeaponDefinitions[] = {
		{
			.id = WeaponTypeSingleLaser,
			.barrelCount = 1,
			.spacing = 0.0f,
			.muzzleOffsetY = 64.0f,
			.secondsPerShot = LaserSecondsPerShot,
			.damage = PlayerLaserDamage,
			.projectileVelocity = glm::vec3(0.0f, -384.0f, 0.0f),
			.projectileSprite = "laserBlue01",
		},
		{
			.id = WeaponTypeDualLaser,
			.barrelCount = 2,
			.spacing = DualLaserSpacing,
			.muzzleOffsetY = 64.0f,
			.secondsPerShot = LaserSecondsPerShot,
			.damage = PlayerLaserDamage,
			.projectileVelocity = glm::vec3(0.0f, -384.0f, 0.0f),
			.projectileSprite = "laserBlue01",
		},
		{
			.id = WeaponTypeEnemyLaser,
			.barrelCount = 1,
			.spacing = 0.0f,
			.muzzleOffsetY = 64.0f,
			.secondsPerShot = EnemyLaserSecondsPerShot,
			.damage = EnemyLaserDamage,
			.projectileVelocity = glm::vec3(0.0f, EnemyLaserSpeedY, 0.0f),
			.projectileSprite = "laserRed01",
		},
		{
			.id = WeaponTypeEnemyHeavyMissile,
			.barrelCount = 1,
			.spacing = 0.0f,
			.muzzleOffsetY = 64.0f,
			.secondsPerShot = EnemyHeavyMissileSecondsPerShot,
			.damage = EnemyHeavyMissileDamage,
			.projectileVelocity = glm::vec3(0.0f, EnemyHeavyMissileSpeedY, 0.0f),
			.projectileSprite = "fire00",
		},
	};

	inline const WeaponDefinition* findWeaponDefinition(std::string_view type)
	{
		if (type.empty())
		{
			return nullptr;
		}

		for (const auto& def : kWeaponDefinitions)
		{
			if (type == def.id)
			{
				return &def;
			}
		}

		return nullptr;
	}

	inline void applyWeapon(WeaponComponent& weapon, std::string_view type)
	{
		const WeaponDefinition* def = findWeaponDefinition(type);
		if (def == nullptr)
		{
			return;
		}

		weapon.Type = def->id;
		weapon.barrelCount = def->barrelCount;
		weapon.spacing = def->spacing;
		weapon.muzzleOffsetY = def->muzzleOffsetY;
		weapon.secondsPerShot = def->secondsPerShot;
		weapon.damage = def->damage;
		weapon.projectileVelocity = def->projectileVelocity;
		weapon.projectileSprite = def->projectileSprite;
	}
}
