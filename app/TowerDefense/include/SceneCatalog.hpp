#pragma once

#include <helsinki/System/glm.hpp>
#include <cmath>
#include <optional>

namespace tower
{
	inline constexpr const char* MarkerTag = "MARKER";
	inline constexpr const char* MarkerModelId = "marker";
	inline constexpr const char* TileTag = "TILE";
	inline constexpr const char* PathTag = "PATH";
	inline constexpr const char* BuildableTag = "BUILDABLE";
	inline constexpr const char* CreepTag = "CREEP";
	inline constexpr const char* CreepModelId = "creep";
	inline constexpr const char* TileBlackModelId = "tile_black";
	inline constexpr const char* TileWhiteModelId = "tile_white";
	inline constexpr const char* TurretModelId = "turret_double";
	inline constexpr const char* TowerTag = "TOWER";
	inline constexpr const char* BlockerTag = "BLOCKER";
	inline constexpr const char* DetailTreeModelId = "detail_tree";
	inline constexpr const char* DetailRocksModelId = "detail_rocks";
	inline constexpr const char* GhostTag = "GHOST";
	inline constexpr const char* GhostOkMaterial = "ghost_ok";
	inline constexpr const char* GhostBadMaterial = "ghost_bad";
	inline constexpr float GhostAlpha = 0.45f;
	inline constexpr const char* RangeRingTag = "RANGE_RING";
	inline constexpr const char* RangeRingModelId = "range_ring";
	inline constexpr float RangeRingY = 0.05f;
	inline constexpr float RangeRingInnerRadius = 0.97f;
	inline constexpr const char* PathRibbonTag = "PATH_RIBBON";
	inline constexpr const char* PathRibbonModelId = "path_ribbon";
	inline constexpr const char* PathRibbonMaterial = "path_ribbon";
	inline constexpr float PathRibbonWidth = 0.2f;
	inline constexpr float PathRibbonY = 0.03f;
	inline constexpr float PathRibbonLength = 7.0f;
	inline constexpr float InvalidFlashSeconds = 0.25f;
	inline constexpr const char* ProjectileTag = "PROJECTILE";
	inline constexpr const char* ProjectileModelId = "marker";
	inline constexpr int StartGold = 100;
	inline constexpr int TowerCost = 25;
	inline constexpr int TowerSellRefund = TowerCost / 2;
	inline constexpr int KillGold = 10;
	inline constexpr int WaveClearBonus = 25;
	inline constexpr int ProjectileDamage = 1;
	inline constexpr int StartLives = 3;
	inline constexpr const char* CreepRunnerMaterial = "creep_runner";
	inline constexpr const char* CreepTankMaterial = "creep_tank";
	struct CreepDef
	{
		const char* material;
		glm::vec3 kd;
		int health;
		float speed;
	};
	inline constexpr CreepDef CreepRunner{
		CreepRunnerMaterial,
		{ 1.0f, 0.45f, 0.15f },
		2,
		2.2f
	};
	inline constexpr CreepDef CreepTank{
		CreepTankMaterial,
		{ 0.35f, 0.45f, 0.85f },
		8,
		0.85f
	};
	struct WaveDef
	{
		int runners;
		int tanks;
	};
	inline constexpr WaveDef Waves[] = {
		{ 5, 0 },
		{ 4, 3 },
		{ 4, 5 },
	};
	inline constexpr int WaveCount = static_cast<int>(sizeof(Waves) / sizeof(Waves[0]));
	inline constexpr int MaxWaveCreeps = 16;
	inline constexpr float WaveSpawnInterval = 1.0f;
	inline constexpr float BuildTimerSeconds = 20.0f;
	inline constexpr float TowerRange = 3.5f;
	inline constexpr float TowerYawOffset = 0.0f;
	inline constexpr float TowerTurnSpeed = 270.0f;
	inline constexpr float TowerFireCooldown = 1.0f;
	inline constexpr float ProjectileSpeed = 6.0f;
	inline constexpr float ProjectileHitRadius = 0.35f;
	inline constexpr float ProjectileY = 0.4f;
	inline constexpr float CameraDistanceMin = 10.0f;
	inline constexpr float CameraDistanceMax = 40.0f;
	inline constexpr float CameraZoomStep = 1.5f;
	inline constexpr float CameraOrbitDegreesPerPixel = 0.25f;
	inline constexpr float CameraSmooth = 12.0f;

	inline constexpr float MarkerY = 0.05f;
	inline constexpr glm::vec3 MarkerScale{ 0.3f, 0.3f, 0.3f };
	inline constexpr glm::vec3 CreepScale{ 0.4f, 0.4f, 0.4f };
	inline constexpr glm::vec3 ProjectileScale{ 0.15f, 0.15f, 0.15f };

	inline constexpr int BoardSize = 8;
	inline constexpr float TileSize = 1.0f;

	struct TileCoord
	{
		int x;
		int z;
	};

	inline constexpr TileCoord PathWaypoints[] = {
		{ 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 }, { 0, 5 }, { 0, 6 }, { 0, 7 },
		{ 1, 7 }, { 2, 7 }, { 3, 7 }, { 4, 7 }, { 5, 7 }, { 6, 7 }, { 7, 7 },
	};

	inline constexpr TileCoord MarkerStartTile{ 3, 3 };

	struct BlockerDef
	{
		const char* model;
		TileCoord tile;
	};

	inline constexpr BlockerDef Blockers[] = {
		{ DetailTreeModelId, { 3, 1 } },
		{ DetailRocksModelId, { 5, 3 } },
		{ DetailTreeModelId, { 6, 2 } },
		{ DetailRocksModelId, { 2, 5 } },
	};

	inline constexpr bool isPathTile(int tx, int tz)
	{
		return tx == 0 || tz == BoardSize - 1;
	}

	inline constexpr bool isOnBoard(int tx, int tz)
	{
		return tx >= 0 && tx < BoardSize && tz >= 0 && tz < BoardSize;
	}

	inline glm::vec3 tileCenter(int tx, int tz, float y = 0.0f)
	{
		const float origin = (BoardSize * TileSize) * 0.5f - TileSize * 0.5f;
		return glm::vec3(
			static_cast<float>(tx) * TileSize - origin,
			y,
			static_cast<float>(tz) * TileSize - origin);
	}

	inline std::optional<TileCoord> worldToTile(const glm::vec3& hit)
	{
		const float half = BoardSize * TileSize * 0.5f;
		const int tx = static_cast<int>(std::floor(hit.x + half));
		const int tz = static_cast<int>(std::floor(hit.z + half));
		if (!isOnBoard(tx, tz))
		{
			return std::nullopt;
		}

		return TileCoord{ tx, tz };
	}
}
