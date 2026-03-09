#pragma once

#include <helsinki/System/glm.hpp>

namespace pong
{
	struct EntityPushConstantObject
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::vec4 color;
	};
}