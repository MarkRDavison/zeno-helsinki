#pragma once

#include <helsinki/System/glm.hpp>

namespace sk
{
	inline constexpr const char* RotateTag = "ROTATE";
	inline constexpr glm::vec3 CameraTarget{ 0.0f, 0.0f, 0.0f };
	inline constexpr float CameraDistanceMin = 1.2f;
	inline constexpr float CameraDistanceMax = 12.0f;
	inline constexpr float CameraZoomStep = 0.4f;
	inline constexpr float CameraOrbitDegreesPerPixel = 0.25f;
	inline constexpr float CameraPitchMin = -89.0f;
	inline constexpr float CameraPitchMax = 89.0f;
	inline constexpr glm::vec3 SceneSunDirection{ 0.45f, 0.85f, 0.30f };
	inline constexpr uint32_t ShadowMapSize = 2048;
	inline constexpr float ShadowOrthoHalfExtent = 4.0f;
	inline constexpr float ShadowLightDistance = 20.0f;
	inline constexpr float ShadowNear = 0.1f;

	struct Prop
	{
		const char* modelId;
		glm::vec3 position;
		bool rotate;
	};

	inline constexpr Prop SceneProps[] = {
		{ "plane", { 0.0f, 0.0f, 0.0f }, false },
		{ "rock_crystals", { -1.0f, 0.0f, -1.0f }, true },
		{ "satelliteDish_detailed", { -1.0f, 0.0f, 1.0f }, true },
		{ "turret_double", { 1.0f, 0.0f, 1.0f }, true },
	};

	struct PointLight
	{
		glm::vec3 position;
		glm::vec3 color;
		float intensity;
		float radius;
	};

	inline constexpr PointLight ScenePointLights[] = {
		{ { -1.0f, 0.35f, 1.0f }, { 1.0f, 0.55f, 0.25f }, 2.0f, 2.5f },
		{ { -1.2f, 0.5f, -1.2f }, { 0.15f, 0.5f, 1.0f }, 4.0f, 3.5f },
	};
}
