#include <catch2/catch_test_macros.hpp>
#include <Ui/UiLayout.hpp>

namespace hur
{
	namespace Test
	{
		TEST_CASE("Top left UI element", "[Hurricane][UiLayout]")
		{
            UiElement element;

            element.size = { 100,50 };
            element.offset = { 10,20 };
            element.anchor = UiAnchor::TopLeft;

            auto rect = UiLayout::Calculate(element, { 800, 600 });

            CHECK(rect.position.x == 10);
            CHECK(rect.position.y == 20);
		}

        TEST_CASE("Centered UI element", "[Hurricane][UiLayout]")
        {
            UiElement element;

            element.size = { 100,50 };
            element.offset = { 0,0 };
            element.anchor = UiAnchor::Center;

            auto rect = UiLayout::Calculate(element, { 800,600 });

            CHECK(rect.position.x == 350);
            CHECK(rect.position.y == 275);
        }
	}
}