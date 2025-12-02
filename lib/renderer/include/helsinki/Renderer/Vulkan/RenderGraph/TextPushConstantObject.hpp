#pragma once

#include <helsinki/System/glm.hpp>

namespace hl
{

	struct alignas(16) TextPushConstantObject
	{
		alignas(16) glm::mat4 model;
	};

}