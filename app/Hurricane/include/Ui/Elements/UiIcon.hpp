#pragma once

#include <string>
#include <Ui/UiElement.hpp>

namespace hur
{

	class UiIcon : public UiElement
	{
	public:
		std::string icon;

		void draw(hl::UiRoot& root, glm::vec2 screenSize) override;
	};

}