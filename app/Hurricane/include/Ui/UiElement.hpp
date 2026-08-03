#pragma once

#include <helsinki/System/glm.hpp>
#include <Ui/UiAnchor.hpp>
#include <Ui/UiRect.hpp>

namespace hur
{

	struct UiElement 
	{
		glm::vec2 size;
		glm::vec2 offset;

		UiAnchor anchor;
		UiRect calculatedRect;
	};

}