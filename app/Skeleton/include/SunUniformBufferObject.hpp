#pragma once

#include <helsinki/System/glm.hpp>

namespace sk
{
	// std140: vec3 + float pack into 16 bytes. GLM aligned vec3 is 16 bytes and
	// would push the following float to 16, which does not match the shader.
	struct SunUniformBufferObject
	{
		glm::vec4 direction{ 0.45f, 0.85f, 0.30f, 1.0f };
		glm::vec4 color{ 1.0f, 0.97f, 0.90f, 0.35f };
		glm::vec4 ambient{ 0.18f, 0.18f, 0.18f, 0.0f };
	};
}
