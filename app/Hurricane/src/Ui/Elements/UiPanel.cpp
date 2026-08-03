#include <Ui/Elements/UiPanel.hpp>

namespace hur
{

	void UiPanel::draw(hl::UiRoot& root, glm::vec2 screenSize)
	{
		root.addQuad(calculatedRect, { colour , 1.0f });
	}

}