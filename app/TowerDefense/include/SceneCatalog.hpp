#pragma once

#include <helsinki/System/glm.hpp>

namespace tower
{
	inline constexpr const char* RotateTag = "ROTATE";
	inline constexpr const char* MarkerTag = "MARKER";
	inline constexpr const char* MarkerModelId = "marker";
	inline constexpr float MarkerY = 0.05f;
	inline constexpr glm::vec3 MarkerScale{ 0.3f, 0.3f, 0.3f };

	struct Prop
	{
		const char* modelId;
		glm::vec3 position;
		bool rotate;
	};

	inline constexpr Prop SceneProps[] = {
		{ "plane", { 0.0f, 0.0f, 0.0f }, false },
		{ "turret_double", { 1.0f, 0.0f, 0.0f }, true },
	};
}
