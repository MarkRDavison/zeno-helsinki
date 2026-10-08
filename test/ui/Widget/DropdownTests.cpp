#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Dropdown.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Panel.hpp>
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
	}

	TEST_CASE("releaseChild detaches and addChild appends last", "[Ui][Dropdown]")
	{
		auto root = std::make_unique<Node>();
		Node& a = root->addChild();
		Node& b = root->addChild();
		CHECK(a.parent() == root.get());
		CHECK(root->children().size() == 2);

		auto held = root->releaseChild(a);
		REQUIRE(held.get() == &a);
		CHECK(a.parent() == nullptr);
		CHECK(root->children().size() == 1);
		CHECK(root->children().front().get() == &b);

		root->addChild(std::move(held));
		CHECK(a.parent() == root.get());
		CHECK(root->children().back().get() == &a);
	}

	TEST_CASE("open overlay is last child of root and does not grow the column", "[Ui][Dropdown]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		auto& column = root->addChild();
		column.kind = Kind::Column;
		column.gap = 8.0f;

		Dropdown dropdown(column.addChild(), typeface);
		dropdown.setItems({ "a", "b", "c" });
		Panel below(column.addChild());
		below.node().intrinsicSize = glm::vec2{ 80.0f, 20.0f };

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		const float belowY = below.node().world.pos.y;

		dropdown.setOpen(true);
		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		CHECK(dropdown.isOpen());
		CHECK(root->children().back().get() == &dropdown.overlay());
		CHECK(below.node().world.pos.y == belowY);
	}

	TEST_CASE("click item selects, notifies, and closes", "[Ui][Dropdown]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		auto& column = root->addChild();
		column.kind = Kind::Column;
		Dropdown dropdown(column.addChild(), typeface);
		dropdown.setItems({ "one", "two", "three" });
		int changed = -1;
		dropdown.onChanged = [&changed](int i) { changed = i; };

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		dropdown.setOpen(true);
		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		REQUIRE_FALSE(dropdown.overlay().children().empty());
		const auto& item = dropdown.overlay().children()[1]->world;
		dispatch(*root, Pointer{ .position = item.pos + glm::vec2{ 8.0f, 8.0f }, .primaryReleased = true });
		CHECK(changed == 1);
		CHECK(dropdown.selectedIndex() == 1);
		CHECK_FALSE(dropdown.isOpen());
	}

	TEST_CASE("hovering an open item updates highlight", "[Ui][Dropdown]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		auto& column = root->addChild();
		column.kind = Kind::Column;
		Dropdown dropdown(column.addChild(), typeface);
		dropdown.setItems({ "one", "two", "three" });

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		dropdown.setOpen(true);
		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		const auto& item = dropdown.overlay().children()[2]->world;
		dispatch(*root, Pointer{ .position = item.pos + glm::vec2{ 8.0f, 8.0f } });
		CHECK(dropdown.highlightIndex() == 2);
		CHECK(dropdown.isOpen());
		CHECK(dropdown.selectedIndex() == 0);
	}

	TEST_CASE("header hover does not stick after the pointer leaves", "[Ui][Dropdown]")
	{
		class RecordingPaint : public IPaint
		{
		public:
			glm::vec4 firstFill{ 0.0f };
			int fills = 0;

			void fill(const Box&, glm::vec4 color) override
			{
				if (fills == 0)
				{
					firstFill = color;
				}

				++fills;
			}

			void sprite(const Box&, glm::vec4, glm::vec3) override {}
			void glyphs(const std::vector<GlyphVertex>&, glm::vec2, glm::vec3) override {}
		};

		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		auto& column = root->addChild();
		column.kind = Kind::Column;
		Dropdown dropdown(column.addChild(), typeface);
		dropdown.setItems({ "one", "two", "three" });

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		const auto onHeader = dropdown.node().world.pos + glm::vec2{ 8.0f, 8.0f };
		dispatch(*root, Pointer{ .position = onHeader, .primaryReleased = true });
		REQUIRE(dropdown.isOpen());

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		const auto& item = dropdown.overlay().children()[1]->world;
		dispatch(*root, Pointer{ .position = item.pos + glm::vec2{ 8.0f, 8.0f } });

		RecordingPaint paint;
		dropdown.paint(paint);
		CHECK(paint.firstFill.x == dropdown.fillColor.x);
		CHECK(paint.firstFill.y == dropdown.fillColor.y);
	}

	TEST_CASE("click outside closes the overlay", "[Ui][Dropdown]")
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

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		dropdown.setOpen(true);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		const auto away = elsewhere.node().world.pos + glm::vec2{ 8.0f, 8.0f };
		dispatch(*root, Pointer{ .position = away, .primaryReleased = true });
		CHECK_FALSE(dropdown.isOpen());
	}

	TEST_CASE("tall list clips to maxListHeight and scrolls", "[Ui][Dropdown]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		auto& column = root->addChild();
		column.kind = Kind::Column;
		Dropdown dropdown(column.addChild(), typeface);
		dropdown.maxListHeight = 80.0f;
		dropdown.setItems({ "a", "b", "c", "d", "e", "f", "g", "h" });

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		dropdown.setOpen(true);
		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		CHECK(dropdown.overlay().world.size.y == 80.0f);
		const auto inside = dropdown.overlay().world.pos + glm::vec2{ 8.0f, 8.0f };
		CHECK(applyScroll(*root, inside, { 0.0f, 20.0f }));
		CHECK(dropdown.overlay().scrollOffset.y == 20.0f);
	}

	TEST_CASE("Enter opens and Down then Enter selects", "[Ui][Dropdown]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		auto& column = root->addChild();
		column.kind = Kind::Column;
		Dropdown dropdown(column.addChild(), typeface);
		dropdown.setItems({ "one", "two", "three" });
		dropdown.setFocused(true);

		dispatchTextKey(*root, TextKey::Enter);
		CHECK(dropdown.isOpen());
		dispatchTextKey(*root, TextKey::Down);
		dispatchTextKey(*root, TextKey::Enter);
		CHECK(dropdown.selectedIndex() == 1);
		CHECK_FALSE(dropdown.isOpen());
	}
}
