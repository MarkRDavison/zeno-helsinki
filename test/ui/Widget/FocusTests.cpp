#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/Checkbox.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/TextField.hpp>
#include <helsinki/Ui/Toggle.hpp>
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
				out.push_back(GlyphVertex{ { width, 30.0f }, { 1.0f, 1.0f } });
				return { width, 30.0f };
			}
		};
	}

	TEST_CASE("Tab wraps focusable widgets in tree order", "[Ui][Focus]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		auto& column = root->addChild();
		column.kind = Kind::Column;
		column.gap = 8.0f;

		Button button(column.addChild(), typeface);
		button.setText("go", 16);
		Checkbox checkbox(column.addChild());
		Toggle toggle(column.addChild());
		TextField field(column.addChild(), typeface);

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		dispatchTextKey(*root, TextKey::Tab);
		CHECK(button.hasKeyboardFocus());
		CHECK_FALSE(checkbox.hasKeyboardFocus());

		dispatchTextKey(*root, TextKey::Tab);
		CHECK(checkbox.hasKeyboardFocus());

		dispatchTextKey(*root, TextKey::Tab);
		CHECK(toggle.hasKeyboardFocus());

		dispatchTextKey(*root, TextKey::Tab);
		CHECK(field.hasKeyboardFocus());

		dispatchTextKey(*root, TextKey::Tab);
		CHECK(button.hasKeyboardFocus());
	}

	TEST_CASE("Shift+Tab wraps in reverse", "[Ui][Focus]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		auto& column = root->addChild();
		column.kind = Kind::Column;

		Button button(column.addChild(), typeface);
		button.setText("go", 16);
		Checkbox checkbox(column.addChild());
		Toggle toggle(column.addChild());
		TextField field(column.addChild(), typeface);

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		dispatchTextKey(*root, TextKey::ShiftTab);
		CHECK(field.hasKeyboardFocus());

		dispatchTextKey(*root, TextKey::ShiftTab);
		CHECK(toggle.hasKeyboardFocus());

		dispatchTextKey(*root, TextKey::ShiftTab);
		CHECK(checkbox.hasKeyboardFocus());

		dispatchTextKey(*root, TextKey::ShiftTab);
		CHECK(button.hasKeyboardFocus());

		dispatchTextKey(*root, TextKey::ShiftTab);
		CHECK(field.hasKeyboardFocus());
	}

	TEST_CASE("Enter on focused Checkbox flips checked", "[Ui][Focus]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Checkbox checkbox(root->addChild());
		checkbox.node().setTopLeft({ 32.0f, 32.0f });
		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		checkbox.setFocused(true);
		CHECK_FALSE(checkbox.checked());
		dispatchTextKey(*root, TextKey::Enter);
		CHECK(checkbox.checked());
	}

	TEST_CASE("Space on focused Button fires onClick", "[Ui][Focus]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Button button(root->addChild(), typeface);
		button.setText("go", 16);
		button.node().setTopLeft({ 80.0f, 30.0f });
		int clicks = 0;
		button.onClick = [&clicks]() { ++clicks; };
		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		button.setFocused(true);
		dispatchTextKey(*root, TextKey::Space);
		CHECK(clicks == 1);
	}

	TEST_CASE("Click TextField then Tab focuses the next sibling", "[Ui][Focus]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		auto& column = root->addChild();
		column.kind = Kind::Column;
		column.gap = 8.0f;

		Checkbox checkbox(column.addChild());
		TextField field(column.addChild(), typeface);
		Button button(column.addChild(), typeface);
		button.setText("go", 16);

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		const auto onField = field.node().world.pos + glm::vec2{ 8.0f, 8.0f };
		dispatch(*root, Pointer{ .position = onField, .primaryReleased = true });
		CHECK(field.hasKeyboardFocus());

		dispatchTextKey(*root, TextKey::Tab);
		CHECK(button.hasKeyboardFocus());
		CHECK_FALSE(field.hasKeyboardFocus());
	}

	TEST_CASE("Tab on an empty or unfocusable tree is a no-op", "[Ui][Focus]")
	{
		auto empty = std::make_unique<Node>();
		empty->setFillParent();
		layout(*empty, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		dispatchTextKey(*empty, TextKey::Tab);
		dispatchTextKey(*empty, TextKey::ShiftTab);

		auto rooted = std::make_unique<Node>();
		rooted->setFillParent();
		Panel panel(rooted->addChild());
		panel.node().setFillParent();
		layout(*rooted, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		dispatchTextKey(*rooted, TextKey::Tab);
		CHECK_FALSE(panel.hasKeyboardFocus());
	}
}
