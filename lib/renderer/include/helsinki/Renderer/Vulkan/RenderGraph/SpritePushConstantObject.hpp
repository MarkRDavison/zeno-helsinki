#pragma once

#include <helsinki/System/glm.hpp>

namespace hl
{

	struct SpritePushConstantObject
	{
		alignas(16) glm::mat4 model;
	};

}