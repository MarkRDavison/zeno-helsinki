#pragma once

#include <helsinki/System/glm.hpp>
#include <Ui/UiAnchor.hpp>
#include <Ui/UiRect.hpp>
#include <Ui/UiRoot.hpp>

namespace hur
{

	class UiElement 
	{
	public:
		glm::vec2 size;
		glm::vec2 offset;

		UiAnchor anchor;
		UiRect calculatedRect;

		virtual void draw(hl::UiRoot& root, glm::vec2 screenSize) = 0;
	};

}