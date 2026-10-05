#pragma once

#include <helsinki/System/glm.hpp>

namespace sk
{
	inline constexpr int MaxPointLights = 4;

	struct PointLightsUniformBufferObject
	{
		glm::ivec4 count{ 0, 0, 0, 0 };
		glm::vec4 positionRadius[MaxPointLights]{};
		glm::vec4 colorIntensity[MaxPointLights]{};
	};
}
