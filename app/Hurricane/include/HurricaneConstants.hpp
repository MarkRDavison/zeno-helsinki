#pragma once

#include <helsinki/System/glm.hpp>

namespace hur
{
	class HurricaneConstants
	{
		HurricaneConstants() = delete;
	public:
		static const constexpr int Width = 800;
		static const constexpr int Height = 600;
	};

	inline constexpr const char* DeathVfxSpriteNames[] = {
		"shield1",
		"shield2",
		"shield3",
	};

	constexpr float DeathVfxSecondsPerFrame = 0.08f;
	constexpr float PickupFallSpeedY = 128.0f;
	constexpr float SpeedBoostDuration = 5.0f;
	constexpr float SpeedBoostMultiplier = 1.5f;
	constexpr float PlayerEngineOverlap = 8.0f;
	inline constexpr const char* PlayerEngineEntityName = "PlayerEngine";
	inline constexpr const char* PlayerEngineSprite = "speed";
	inline constexpr const char* PickupIdSpeed = "powerupBlue";
	inline constexpr const char* PickupIdShield = "powerupBlue_shield";
	constexpr int PlayerShieldMaxLayers = 3;
	inline constexpr const char* PlayerShieldEntityName = "PlayerShield";
	inline constexpr const char* PlayerShieldSpriteNames[] = {
		"shield1",
		"shield2",
		"shield3",
	};
	constexpr float DualLaserSpacing = 24.0f;
	constexpr float LaserSecondsPerShot = 0.15f;
	constexpr float EnemyLaserSecondsPerShot = 1.5f;
	constexpr float EnemyLaserSpeedY = 192.0f;
	constexpr float EnemyHeavyMissileSecondsPerShot = 2.0f;
	constexpr float EnemyHeavyMissileSpeedY = 192.0f;
	constexpr int PlayerLaserDamage = 3;
	constexpr int EnemyLaserDamage = 1;
	constexpr int EnemyHeavyMissileDamage = 5;
	constexpr float BombAoeRadius = 200.0f;
	constexpr float BombBlastSecondsPerFrame = 0.08f;
	constexpr float BombMissileMuzzleOffsetY = 64.0f;
	constexpr glm::vec3 BombMissileVelocity{ 0.0f, -320.0f, 0.0f };
	inline constexpr const char* BombMissileSprite = "fire00";

	inline constexpr const char* BombBlastSpriteNames[] = {
		"star1",
		"star2",
		"star3",
	};

	enum class GameState
	{
		INIT = 0,
		PLAYING = 1,
		GAME_OVER = 2,
		PAUSED = 3
	};

	constexpr int UI_TEX_WHITE = 0;
	constexpr int UI_TEX_ROBOTO = 1;
	constexpr int UI_TEX_SHEET = 2;
}