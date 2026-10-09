#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/Dropdown.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Tabs.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <algorithm>
#include <memory>
#include <string>

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
				const float width = std::max(40.0f, static_cast<float>(text.size()) * 10.0f);
				out.push_back(GlyphVertex{ { 0.0f, 0.0f }, { 0.0f, 0.0f } });
				out.push_back(GlyphVertex{ { width, 16.0f }, { 1.0f, 1.0f } });
				return { width, 16.0f };
			}
		};
	}

	TEST_CASE("clicking a tab header selects exclusively", "[Ui][Tabs]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Tabs tabs(root->addChild(), typeface);
		tabs.addPage("A");
		tabs.addPage("B");
		CHECK(tabs.pageCount() == 2);
		CHECK(tabs.selectedIndex() == 0);

		int changed = -1;
		int fires = 0;
		tabs.onChanged = [&](int i)
		{
			changed = i;
			++fires;
		};

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		REQUIRE(tabs.headerClip().children().size() == 2);

		const auto onB = tabs.headerClip().children()[1]->world.pos + glm::vec2{ 8.0f, 8.0f };
		dispatch(*root, Pointer{ .position = onB, .primaryReleased = true });
		CHECK(tabs.selectedIndex() == 1);
		CHECK(changed == 1);
		CHECK(fires == 1);

		dispatch(*root, Pointer{ .position = onB, .primaryReleased = true });
		CHECK(tabs.selectedIndex() == 1);
		CHECK(fires == 1);
	}

	TEST_CASE("hidden page has height 0 and selected page does not", "[Ui][Tabs]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Tabs tabs(root->addChild(), typeface);
		Button onA(tabs.addPage("A").addChild(), typeface);
		onA.setText("first");
		Button onB(tabs.addPage("B").addChild(), typeface);
		onB.setText("second");

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		CHECK(tabs.page(0).world.size.y > 0.0f);
		CHECK(tabs.page(1).world.size.y == 0.0f);

		tabs.pick(1);
		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		CHECK(tabs.page(0).world.size.y == 0.0f);
		CHECK(tabs.page(1).world.size.y > 0.0f);
	}

	TEST_CASE("collectFocusables skips hidden page widgets", "[Ui][Tabs]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Tabs tabs(root->addChild(), typeface);
		Button onA(tabs.addPage("A").addChild(), typeface);
		onA.setText("first");
		Button onB(tabs.addPage("B").addChild(), typeface);
		onB.setText("second");

		std::vector<Widget*> focusables;
		collectFocusables(*root, focusables);
		REQUIRE(focusables.size() == 2);
		CHECK(focusables[0] == &tabs);
		CHECK(focusables[1] == &onA);

		tabs.pick(1);
		focusables.clear();
		collectFocusables(*root, focusables);
		REQUIRE(focusables.size() == 2);
		CHECK(focusables[0] == &tabs);
		CHECK(focusables[1] == &onB);
	}

	TEST_CASE("focused arrows move tabs and wrap", "[Ui][Tabs]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Tabs tabs(root->addChild(), typeface);
		tabs.addPage("A");
		tabs.addPage("B");
		tabs.addPage("C");
		tabs.setFocused(true);

		dispatchTextKey(*root, TextKey::Right);
		CHECK(tabs.selectedIndex() == 1);
		dispatchTextKey(*root, TextKey::Down);
		CHECK(tabs.selectedIndex() == 2);
		dispatchTextKey(*root, TextKey::Right);
		CHECK(tabs.selectedIndex() == 0);
		dispatchTextKey(*root, TextKey::Left);
		CHECK(tabs.selectedIndex() == 2);
	}

	TEST_CASE("overflow shows chevrons that scroll the header clip", "[Ui][Tabs]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Tabs tabs(root->addChild(), typeface);
		tabs.maxHeaderWidth = 80.0f;
		for (int i = 0; i < 8; ++i)
		{
			tabs.addPage("Tab" + std::to_string(i));
		}

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		REQUIRE(tabs.chevronsVisible());
		CHECK(tabs.headerClip().scrollOffset.x == 0.0f);

		const auto& bar = *tabs.node().children()[0];
		REQUIRE(bar.children().size() == 3);
		const auto onNext = bar.children()[2]->world.pos + glm::vec2{ 4.0f, 4.0f };
		dispatch(*root, Pointer{ .position = onNext, .primaryReleased = true });
		CHECK(tabs.headerClip().scrollOffset.x > 0.0f);

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		CHECK(tabs.headerClip().scrollOffset.x > 0.0f);

		const float scrolled = tabs.headerClip().scrollOffset.x;
		const auto onPrev = bar.children()[0]->world.pos + glm::vec2{ 4.0f, 4.0f };
		dispatch(*root, Pointer{ .position = onPrev, .primaryReleased = true });
		CHECK(tabs.headerClip().scrollOffset.x < scrolled);
	}

	TEST_CASE("few tabs hide chevrons", "[Ui][Tabs]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Tabs tabs(root->addChild(), typeface);
		tabs.addPage("A");
		tabs.addPage("B");

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		CHECK_FALSE(tabs.chevronsVisible());
		CHECK(tabs.headerClip().scrollOffset.x == 0.0f);
	}

	TEST_CASE("dropdown click-outside still works with Tabs in the tree", "[Ui][Tabs]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		auto& column = root->addChild();
		column.kind = Kind::Column;
		Dropdown dropdown(column.addChild(), typeface);
		dropdown.setItems({ "one", "two" });
		Tabs tabs(column.addChild(), typeface);
		tabs.addPage("A");
		Panel elsewhere(root->addChild());
		elsewhere.node().setBottomRight({ 40.0f, 40.0f });

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		dropdown.setOpen(true);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		const auto away = elsewhere.node().world.pos + glm::vec2{ 8.0f, 8.0f };
		dispatch(*root, Pointer{ .position = away, .primaryReleased = true });
		CHECK_FALSE(dropdown.isOpen());
	}
}
