#include <Ui/UiLayout.hpp>
#include <cassert>

namespace hur
{
	UiRect UiLayout::Calculate(const UiElement& element, glm::vec2 screenSize)
	{
        glm::vec2 position{ 0 };

        switch (element.anchor)
        {
        case UiAnchor::TopLeft:
            position = {
                0,
                0
            };
            break;


        case UiAnchor::Center:
            position = {
                (screenSize.x - element.size.x) * 0.5f,
                (screenSize.y - element.size.y) * 0.5f
            };
            break;

        default:
            assert(false);
        }

        position += element.offset;

        return {
            position,
            element.size
        };
	}
}