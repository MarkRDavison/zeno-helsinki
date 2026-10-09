#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/Dropdown.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Theme.hpp>
#include <helsinki/Ui/Tooltip.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <algorithm>
#include <memory>

namespace hl::ui::test
{
	namespace
	{
		class FakeTypeface : public ITypeface
		{
		public:
			glm::vec2 layoutText(
				std::string_view text,
				unsigned,
				std::vector<GlyphVertex>& out) const override
			{
				out.clear();
				const float width = std::max(80.0f, static_cast<float>(text.size()) * 10.0f);
				out.push_back(GlyphVertex{ { 0.0f, 0.0f }, { 0.0f, 0.0f } });
				out.push_back(GlyphVertex{ { width, 20.0f }, { 1.0f, 1.0f } });
				return { width, 20.0f };
			}
		};

		struct ThemeDelayGuard
		{
			float previous;

			explicit ThemeDelayGuard(float delay) :
				previous(theme().tooltipDelay)
			{
				theme().tooltipDelay = delay;
			}

			~ThemeDelayGuard()
			{
				theme().tooltipDelay = previous;
			}
		};

		struct ThemeOffsetGuard
		{
			float previous;

			explicit ThemeOffsetGuard(float offset) :
				previous(theme().tooltipOffset)
			{
				theme().tooltipOffset = offset;
			}

			~ThemeOffsetGuard()
			{
				theme().tooltipOffset = previous;
			}
		};

		struct ThemePlacementGuard
		{
			TooltipPlacement previous;

			explicit ThemePlacementGuard(TooltipPlacement placement) :
				previous(theme().tooltipPlacement)
			{
				theme().tooltipPlacement = placement;
			}

			~ThemePlacementGuard()
			{
				theme().tooltipPlacement = previous;
			}
		};

		void showZeroDelay(Node& root, Tooltip& tooltip, Button& button)
		{
			prepareTree(root);
			layout(root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
			const auto onButton = button.node().world.pos + glm::vec2{ 8.0f, 8.0f };
			dispatch(root, Pointer{ .position = onButton });
			tooltip.tick(0.0f);
			REQUIRE(tooltip.isVisible());
		}
	}

	TEST_CASE("delay 0 shows the overlay on hover", "[Ui][Tooltip]")
	{
		ThemeDelayGuard guard(0.4f);
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Button button(root->addChild(), typeface);
		button.setText("go");
		button.tooltip = "tip";
		Tooltip tooltip(root->addChild(), typeface);
		tooltip.delay = 0.0f;

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		const auto onButton = button.node().world.pos + glm::vec2{ 8.0f, 8.0f };
		dispatch(*root, Pointer{ .position = onButton });
		tooltip.tick(0.0f);

		CHECK(tooltip.isVisible());
		CHECK(root->children().back().get() == &tooltip.node());
		CHECK(tooltip.node().world.size.y > 0.0f);
	}

	TEST_CASE("pointer elsewhere hides the overlay immediately", "[Ui][Tooltip]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Button button(root->addChild(), typeface);
		button.setText("go");
		button.tooltip = "tip";
		Panel elsewhere(root->addChild());
		elsewhere.node().setBottomRight({ 40.0f, 40.0f });
		Tooltip tooltip(root->addChild(), typeface);
		tooltip.delay = 0.0f;

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		const auto onButton = button.node().world.pos + glm::vec2{ 8.0f, 8.0f };
		dispatch(*root, Pointer{ .position = onButton });
		tooltip.tick(0.0f);
		REQUIRE(tooltip.isVisible());

		const auto away = elsewhere.node().world.pos + glm::vec2{ 8.0f, 8.0f };
		dispatch(*root, Pointer{ .position = away });
		tooltip.tick(0.0f);
		CHECK_FALSE(tooltip.isVisible());
		CHECK(tooltip.node().world.size.y == 0.0f);
	}

	TEST_CASE("theme delay waits before showing", "[Ui][Tooltip]")
	{
		ThemeDelayGuard guard(0.4f);
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Button button(root->addChild(), typeface);
		button.setText("go");
		button.tooltip = "tip";
		Tooltip tooltip(root->addChild(), typeface);

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		const auto onButton = button.node().world.pos + glm::vec2{ 8.0f, 8.0f };
		dispatch(*root, Pointer{ .position = onButton });
		tooltip.tick(0.2f);
		CHECK_FALSE(tooltip.isVisible());

		tooltip.tick(0.3f);
		CHECK(tooltip.isVisible());
		CHECK(tooltip.node().world.size.y > 0.0f);
	}

	TEST_CASE("instance delay 0 ignores a non-zero theme delay", "[Ui][Tooltip]")
	{
		ThemeDelayGuard guard(0.4f);
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Button button(root->addChild(), typeface);
		button.setText("go");
		button.tooltip = "tip";
		Tooltip tooltip(root->addChild(), typeface);
		tooltip.delay = 0.0f;

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		const auto onButton = button.node().world.pos + glm::vec2{ 8.0f, 8.0f };
		dispatch(*root, Pointer{ .position = onButton });
		CHECK(tooltip.isVisible());
		CHECK(tooltip.resolvedDelay() == 0.0f);
		CHECK(theme().tooltipDelay == 0.4f);
	}

	TEST_CASE("collectFocusables omits Tooltip", "[Ui][Tooltip]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		Button button(root->addChild(), typeface);
		button.setText("go");
		Tooltip tooltip(root->addChild(), typeface);

		std::vector<Widget*> focusables;
		collectFocusables(*root, focusables);
		CHECK(focusables.size() == 1);
		CHECK(focusables[0] == &button);
	}

	TEST_CASE("dropdown click-outside still works with a Tooltip in the tree", "[Ui][Tooltip]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		auto& column = root->addChild();
		column.kind = Kind::Column;
		Dropdown dropdown(column.addChild(), typeface);
		dropdown.setItems({ "one", "two" });
		Panel elsewhere(root->addChild());
		elsewhere.node().setBottomRight({ 40.0f, 40.0f });
		Tooltip tooltip(root->addChild(), typeface);

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		dropdown.setOpen(true);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		const auto away = elsewhere.node().world.pos + glm::vec2{ 8.0f, 8.0f };
		dispatch(*root, Pointer{ .position = away, .primaryReleased = true });
		CHECK_FALSE(dropdown.isOpen());
	}

	TEST_CASE("theme offset is the gap from the target", "[Ui][Tooltip]")
	{
		ThemeOffsetGuard guard(24.0f);
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Button button(root->addChild(), typeface);
		button.setText("go");
		button.tooltip = "tip";
		Tooltip tooltip(root->addChild(), typeface);
		tooltip.delay = 0.0f;

		showZeroDelay(*root, tooltip, button);
		CHECK(tooltip.resolvedOffset() == 24.0f);
		CHECK(tooltip.node().world.pos.y == button.node().world.pos.y + button.node().world.size.y + 24.0f);
	}

	TEST_CASE("instance offset ignores theme offset", "[Ui][Tooltip]")
	{
		ThemeOffsetGuard guard(8.0f);
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Button button(root->addChild(), typeface);
		button.setText("go");
		button.tooltip = "tip";
		Tooltip tooltip(root->addChild(), typeface);
		tooltip.delay = 0.0f;
		tooltip.offset = 32.0f;

		showZeroDelay(*root, tooltip, button);
		CHECK(tooltip.resolvedOffset() == 32.0f);
		CHECK(theme().tooltipOffset == 8.0f);
		CHECK(tooltip.node().world.pos.y == button.node().world.pos.y + button.node().world.size.y + 32.0f);
	}

	TEST_CASE("theme placement Above sits above the target", "[Ui][Tooltip]")
	{
		ThemePlacementGuard guard(TooltipPlacement::Above);
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Button button(root->addChild(), typeface);
		button.setText("go");
		button.tooltip = "tip";
		button.node().setCenter({ 120.0f, 32.0f });
		Tooltip tooltip(root->addChild(), typeface);
		tooltip.delay = 0.0f;

		showZeroDelay(*root, tooltip, button);
		CHECK(tooltip.resolvedPlacement() == TooltipPlacement::Above);
		CHECK(tooltip.node().world.pos.y + tooltip.node().world.size.y + tooltip.resolvedOffset()
			== button.node().world.pos.y);
	}

	TEST_CASE("instance placement ignores theme placement", "[Ui][Tooltip]")
	{
		ThemePlacementGuard guard(TooltipPlacement::Above);
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Button button(root->addChild(), typeface);
		button.setText("go");
		button.tooltip = "tip";
		Tooltip tooltip(root->addChild(), typeface);
		tooltip.delay = 0.0f;
		tooltip.placement = TooltipPlacement::Right;

		showZeroDelay(*root, tooltip, button);
		CHECK(tooltip.resolvedPlacement() == TooltipPlacement::Right);
		CHECK(theme().tooltipPlacement == TooltipPlacement::Above);
		CHECK(tooltip.node().world.pos.x == button.node().world.pos.x + button.node().world.size.x
			+ tooltip.resolvedOffset());
	}
}
