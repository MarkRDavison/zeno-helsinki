#pragma once

#include <helsinki/System/glm.hpp>
#include <string>

namespace hl
{

	struct Material
	{
		std::string name;
		glm::vec3 diffuse{ 1.0f, 1.0f, 1.0f };
		std::string diffuseTex;
	};

}