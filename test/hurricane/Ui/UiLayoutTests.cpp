#include <catch2/catch_test_macros.hpp>
#include <Ui/UiLayout.hpp>

namespace hur
{
	namespace Test
	{
        TEST_CASE("Top left UI element", "[Hurricane][UiLayout]")
        {
            UiElement element;

            element.size = { 100, 50 };
            element.offset = { 10, 20 };
            element.anchor = UiAnchor::TopLeft;

            auto rect = UiLayout::Calculate(element, { 800, 600 });

            CHECK(rect.position.x == 10);
            CHECK(rect.position.y == 20);
        }

        TEST_CASE("Top center UI element", "[Hurricane][UiLayout]")
        {
            UiElement element;

            element.size = { 100, 50 };
            element.offset = { 10, 20 };
            element.anchor = UiAnchor::TopCenter;

            auto rect = UiLayout::Calculate(element, { 800, 600 });

            CHECK(rect.position.x == 360);
            CHECK(rect.position.y == 20);
        }

        TEST_CASE("Top right UI element", "[Hurricane][UiLayout]")
        {
            UiElement element;

            element.size = { 100, 50 };
            element.offset = { 10, 20 };
            element.anchor = UiAnchor::TopRight;

            auto rect = UiLayout::Calculate(element, { 800, 600 });

            CHECK(rect.position.x == 690);
            CHECK(rect.position.y == 20);
        }

        TEST_CASE("Center left UI element", "[Hurricane][UiLayout]")
        {
            UiElement element;

            element.size = { 100, 50 };
            element.offset = { 10, 20 };
            element.anchor = UiAnchor::CenterLeft;

            auto rect = UiLayout::Calculate(element, { 800, 600 });

            CHECK(rect.position.x == 10);
            CHECK(rect.position.y == 295);
        }

        TEST_CASE("Centered UI element", "[Hurricane][UiLayout]")
        {
            UiElement element;

            element.size = { 100, 50 };
            element.offset = { 10, 20 };
            element.anchor = UiAnchor::Center;

            auto rect = UiLayout::Calculate(element, { 800, 600 });

            CHECK(rect.position.x == 360);
            CHECK(rect.position.y == 295);
        }

        TEST_CASE("Center right UI element", "[Hurricane][UiLayout]")
        {
            UiElement element;

            element.size = { 100, 50 };
            element.offset = { 10, 20 };
            element.anchor = UiAnchor::CenterRight;

            auto rect = UiLayout::Calculate(element, { 800, 600 });

            CHECK(rect.position.x == 690);
            CHECK(rect.position.y == 295);
        }

        TEST_CASE("Bottom left UI element", "[Hurricane][UiLayout]")
        {
            UiElement element;

            element.size = { 100, 50 };
            element.offset = { 10, 20 };
            element.anchor = UiAnchor::BottomLeft;

            auto rect = UiLayout::Calculate(element, { 800, 600 });

            CHECK(rect.position.x == 10);
            CHECK(rect.position.y == 530);
        }

        TEST_CASE("Bottom center UI element", "[Hurricane][UiLayout]")
        {
            UiElement element;

            element.size = { 100, 50 };
            element.offset = { 10, 20 };
            element.anchor = UiAnchor::BottomCenter;

            auto rect = UiLayout::Calculate(element, { 800, 600 });

            CHECK(rect.position.x == 360);
            CHECK(rect.position.y == 530);
        }

        TEST_CASE("Bottom right UI element", "[Hurricane][UiLayout]")
        {
            UiElement element;

            element.size = { 100, 50 };
            element.offset = { 10, 20 };
            element.anchor = UiAnchor::BottomRight;

            auto rect = UiLayout::Calculate(element, { 800, 600 });

            CHECK(rect.position.x == 690);
            CHECK(rect.position.y == 530);
        }
	}
}