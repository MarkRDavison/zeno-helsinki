#pragma once

#include <helsinki/System/glm.hpp>

namespace tower
{
	inline constexpr const char* TileTag = "TILE";
	inline constexpr const char* PathTag = "PATH";
	inline constexpr const char* BuildableTag = "BUILDABLE";
	inline constexpr const char* CreepTag = "CREEP";
	inline constexpr const char* TileBlackModelId = "tile_black";
	inline constexpr const char* TileWhiteModelId = "tile_white";
	inline constexpr const char* TowerTag = "TOWER";
	inline constexpr const char* GhostTag = "GHOST";
	inline constexpr const char* GhostOkMaterial = "ghost_ok";
	inline constexpr const char* GhostBadMaterial = "ghost_bad";
	inline constexpr float GhostAlpha = 0.45f;
	inline constexpr const char* RangeRingTag = "RANGE_RING";
	inline constexpr const char* RangeRingModelId = "range_ring";
	inline constexpr float RangeRingY = 0.05f;
	inline constexpr const char* StatusRingTag = "STATUS_RING";
	inline constexpr float StatusRingY = 0.2f;
	inline constexpr float RangeRingInnerRadius = 0.97f;
	inline constexpr const char* PathRibbonTag = "PATH_RIBBON";
	inline constexpr const char* PathRibbonModelId = "path_ribbon";
	inline constexpr const char* PathRibbonMaterial = "path_ribbon";
	inline constexpr float PathRibbonWidth = 0.2f;
	inline constexpr float PathRibbonY = 0.03f;
	inline constexpr glm::vec3 PathRibbonPalette[] = {
		{ 0.95f, 0.65f, 0.15f },
		{ 0.20f, 0.75f, 0.90f },
		{ 0.85f, 0.35f, 0.80f },
		{ 0.40f, 0.85f, 0.35f },
		{ 0.95f, 0.40f, 0.35f },
		{ 0.55f, 0.55f, 0.95f }
	};
	inline constexpr const char* TargetLineTag = "TARGET_LINE";
	inline constexpr const char* TargetLineCreepMaterial = "target_line_creep";
	inline constexpr const char* TargetLineFocusMaterial = "target_line_focus";
	inline constexpr float TargetLineWidth = 0.08f;
	inline constexpr float TargetLineY = 0.45f;
	inline constexpr glm::vec3 TargetLineCreepColor{ 0.95f, 0.85f, 0.20f };
	inline constexpr glm::vec3 TargetLineFocusColor{ 0.95f, 0.25f, 0.85f };
	inline constexpr float InvalidFlashSeconds = 0.25f;
	inline constexpr const char* ProjectileTag = "PROJECTILE";
	inline constexpr int towerSellRefund(int cost)
	{
		return cost / 2;
	}
	inline constexpr float TowerYawOffset = 0.0f;
	inline constexpr float TowerTurnSpeed = 270.0f;
	inline constexpr float CameraDistanceMin = 10.0f;
	inline constexpr float CameraDistanceMax = 40.0f;
	inline constexpr float CameraZoomStep = 1.5f;
	inline constexpr float CameraOrbitDegreesPerPixel = 0.25f;
	inline constexpr float CameraSmooth = 12.0f;
	inline constexpr glm::vec3 SceneSunDirection{ 0.45f, 0.85f, 0.30f };
	inline constexpr uint32_t ShadowMapSize = 2048;
	inline constexpr float ShadowOrthoMargin = 3.0f;
	inline constexpr float ShadowLightDistance = 20.0f;
	inline constexpr float ShadowNear = 0.1f;

	inline constexpr glm::vec3 CreepScale{ 0.4f, 0.4f, 0.4f };

	inline constexpr float TileSize = 1.0f;

	struct TileCoord
	{
		int x;
		int z;
	};
}
