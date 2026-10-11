#pragma once

#include <helsinki/System/glm.hpp>
#include <cstdint>

namespace slr
{

	struct GalaxyPushConstantObject
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::vec4 color;
		alignas(4) int cameraIndex{ 0 };
		uint32_t pad[3]{};
	};

}
