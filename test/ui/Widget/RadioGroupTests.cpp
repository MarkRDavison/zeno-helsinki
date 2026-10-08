#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/RadioGroup.hpp>
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
				const float width = std::max(40.0f, static_cast<float>(text.size()) * 10.0f);
				out.push_back(GlyphVertex{ { 0.0f, 0.0f }, { 0.0f, 0.0f } });
				out.push_back(GlyphVertex{ { width, 16.0f }, { 1.0f, 1.0f } });
				return { width, 16.0f };
			}
		};
	}

	TEST_CASE("RadioGroup orientation sets Column or Row", "[Ui][RadioGroup]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		RadioGroup group(root->addChild(), typeface);
		group.setItems({ "A", "B" });
		CHECK(group.node().kind == Kind::Column);

		group.setOrientation(RadioOrientation::Horizontal);
		CHECK(group.node().kind == Kind::Row);

		group.setOrientation(RadioOrientation::Vertical);
		CHECK(group.node().kind == Kind::Column);
	}

	TEST_CASE("clicking another option selects exclusively", "[Ui][RadioGroup]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		RadioGroup group(root->addChild(), typeface);
		group.setItems({ "A", "B", "C" });
		int changed = -1;
		int fires = 0;
		group.onChanged = [&](int i)
		{
			changed = i;
			++fires;
		};

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		REQUIRE(group.node().children().size() == 3);

		const auto onB = group.node().children()[1]->world.pos + glm::vec2{ 8.0f, 8.0f };
		dispatch(*root, Pointer{ .position = onB, .primaryReleased = true });
		CHECK(group.selectedIndex() == 1);
		CHECK(changed == 1);
		CHECK(fires == 1);

		dispatch(*root, Pointer{ .position = onB, .primaryReleased = true });
		CHECK(group.selectedIndex() == 1);
		CHECK(fires == 1);
	}

	TEST_CASE("horizontal pack is shorter than vertical for the same items", "[Ui][RadioGroup]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		auto& column = root->addChild();
		column.kind = Kind::Column;

		RadioGroup group(column.addChild(), typeface);
		group.setItems({ "A", "B", "C" });
		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		const float verticalH = group.node().world.size.y;

		group.setOrientation(RadioOrientation::Horizontal);
		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		CHECK(group.node().kind == Kind::Row);
		CHECK(group.node().world.size.y < verticalH);
	}

	TEST_CASE("focused arrows move selection and wrap", "[Ui][RadioGroup]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		RadioGroup group(root->addChild(), typeface);
		group.setItems({ "A", "B", "C" });
		group.setFocused(true);

		dispatchTextKey(*root, TextKey::Down);
		CHECK(group.selectedIndex() == 1);
		dispatchTextKey(*root, TextKey::Right);
		CHECK(group.selectedIndex() == 2);
		dispatchTextKey(*root, TextKey::Down);
		CHECK(group.selectedIndex() == 0);
		dispatchTextKey(*root, TextKey::Up);
		CHECK(group.selectedIndex() == 2);
	}

	TEST_CASE("only the group is focusable, not options", "[Ui][RadioGroup]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		RadioGroup group(root->addChild(), typeface);
		group.setItems({ "A", "B" });

		std::vector<Widget*> focusables;
		collectFocusables(*root, focusables);
		REQUIRE(focusables.size() == 1);
		CHECK(focusables[0] == &group);
	}
}
