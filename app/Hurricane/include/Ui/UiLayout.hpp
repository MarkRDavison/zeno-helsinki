#pragma once

#include <Ui/UiElement.hpp>
#include <Ui/UiRect.hpp>

namespace hur
{

	class UiLayout
	{
		UiLayout() = delete;
	public:
		static UiRect Calculate(const UiElement& element, glm::vec2 screenSize);

	};

}