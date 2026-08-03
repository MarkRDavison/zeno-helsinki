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
                element.offset.x,
                element.offset.y
            };
            break;

        case UiAnchor::TopCenter:
            position = {
                (screenSize.x - element.size.x) * 0.5f + element.offset.x,
                element.offset.y
            };
            break;

        case UiAnchor::TopRight:
            position = {
                screenSize.x - element.size.x - element.offset.x,
                element.offset.y
            };
            break;

        case UiAnchor::CenterLeft:
            position = {
                element.offset.x,
                (screenSize.y - element.size.y) * 0.5f + element.offset.y
            };
            break;

        case UiAnchor::Center:
            position = {
                (screenSize.x - element.size.x) * 0.5f + element.offset.x,
                (screenSize.y - element.size.y) * 0.5f + element.offset.y
            };
            break;

        case UiAnchor::CenterRight:
            position = {
                screenSize.x - element.size.x - element.offset.x,
                (screenSize.y - element.size.y) * 0.5f + element.offset.y
            };
            break;

        case UiAnchor::BottomLeft:
            position = {
                element.offset.x,
                screenSize.y - element.size.y - element.offset.y
            };
            break;

        case UiAnchor::BottomCenter:
            position = {
                (screenSize.x - element.size.x) * 0.5f + element.offset.x,
                screenSize.y - element.size.y - element.offset.y
            };
            break;

        case UiAnchor::BottomRight:
            position = {
                screenSize.x - element.size.x - element.offset.x,
                screenSize.y - element.size.y - element.offset.y
            };
            break;

        default:
            assert(false);
        }

        return {
            position,
            element.size
        };
    }
}