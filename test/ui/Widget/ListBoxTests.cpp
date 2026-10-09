#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/Dropdown.hpp>
#include <helsinki/Ui/Label.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/ListBox.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

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

	TEST_CASE("setItems builds one content child per item", "[Ui][ListBox]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		ListBox list(root->addChild());
		list.setItems(std::vector<std::string>{ "a", "b", "c" }, [&](Node& row, const std::string& text)
		{
			auto label = std::make_unique<Label>(row, typeface);
			label->setText(text);
			return label;
		});

		CHECK(list.itemCount() == 3);
		CHECK(list.scroll().content().children().size() == 3);
	}

	TEST_CASE("setItems replaces previous rows", "[Ui][ListBox]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		ListBox list(root->addChild());
		list.setItems(std::vector<std::string>{ "a", "b", "c" }, [&](Node& row, const std::string& text)
		{
			auto label = std::make_unique<Label>(row, typeface);
			label->setText(text);
			return label;
		});
		list.setItems(std::vector<std::string>{ "x" }, [&](Node& row, const std::string& text)
		{
			auto label = std::make_unique<Label>(row, typeface);
			label->setText(text);
			return label;
		});

		CHECK(list.itemCount() == 1);
		CHECK(list.scroll().content().children().size() == 1);
	}

	TEST_CASE("many items scroll through the inner ScrollView", "[Ui][ListBox]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		ListBox list(root->addChild());
		list.viewportSize = { 100.0f, 80.0f };
		std::vector<std::string> names;
		for (int i = 0; i < 12; ++i)
		{
			names.push_back("row" + std::to_string(i));
		}

		list.setItems(names, [&](Node& row, const std::string& text)
		{
			auto label = std::make_unique<Label>(row, typeface);
			label->setText(text);
			return label;
		});

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		REQUIRE(list.scroll().viewport().maxScroll().y > 0.0f);

		const auto inside = list.scroll().viewport().world.pos + glm::vec2{ 8.0f, 8.0f };
		REQUIRE(applyScroll(*root, inside, { 0.0f, 20.0f }));
		CHECK(list.scroll().viewport().scrollOffset.y == 20.0f);
	}

	TEST_CASE("scrollBars false hides the bar and wheel still works", "[Ui][ListBox]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		ListBox list(root->addChild());
		list.viewportSize = { 100.0f, 80.0f };
		list.scrollBars = false;
		std::vector<std::string> names;
		for (int i = 0; i < 12; ++i)
		{
			names.push_back("row" + std::to_string(i));
		}

		list.setItems(names, [&](Node& row, const std::string& text)
		{
			auto label = std::make_unique<Label>(row, typeface);
			label->setText(text);
			return label;
		});

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		CHECK_FALSE(list.scroll().barVisible());

		const auto inside = list.scroll().viewport().world.pos + glm::vec2{ 8.0f, 8.0f };
		REQUIRE(applyScroll(*root, inside, { 0.0f, 20.0f }));
		CHECK(list.scroll().viewport().scrollOffset.y == 20.0f);
	}

	TEST_CASE("collectFocusables skips ListBox but keeps Button rows", "[Ui][ListBox]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		ListBox labels(root->addChild());
		labels.setItems(std::vector<std::string>{ "a", "b" }, [&](Node& row, const std::string& text)
		{
			auto label = std::make_unique<Label>(row, typeface);
			label->setText(text);
			return label;
		});

		std::vector<Widget*> focusables;
		collectFocusables(*root, focusables);
		CHECK(focusables.empty());

		ListBox buttons(root->addChild());
		buttons.setItems(std::vector<std::string>{ "go" }, [&](Node& row, const std::string& text)
		{
			auto button = std::make_unique<Button>(row, typeface);
			button->setText(text);
			return button;
		});

		focusables.clear();
		collectFocusables(*root, focusables);
		REQUIRE(focusables.size() == 1);
		CHECK(focusables[0]->focusable);
	}

	TEST_CASE("dropdown click-outside still works with a ListBox in the tree", "[Ui][ListBox]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		auto& column = root->addChild();
		column.kind = Kind::Column;
		Dropdown dropdown(column.addChild(), typeface);
		dropdown.setItems({ "one", "two" });
		ListBox list(column.addChild());
		list.setItems(std::vector<std::string>{ "a" }, [&](Node& row, const std::string& text)
		{
			auto label = std::make_unique<Label>(row, typeface);
			label->setText(text);
			return label;
		});
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
