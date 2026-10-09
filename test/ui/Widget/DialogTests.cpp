#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/Dialog.hpp>
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

		class RecordingPaint : public IPaint
		{
		public:
			int fills = 0;

			void fill(const Box&, glm::vec4) override
			{
				++fills;
			}

			void sprite(const Box&, glm::vec4, glm::vec3) override {}
			void glyphs(const std::vector<GlyphVertex>&, glm::vec2, glm::vec3) override {}
		};

		void settle(Node& root)
		{
			prepareTree(root);
			layout(root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		}
	}

	TEST_CASE("closed dialog is skipped by focus and paint", "[Ui][Dialog]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Dialog dialog(*root, typeface);

		settle(*root);

		std::vector<Widget*> focusables;
		collectFocusables(*root, focusables);
		CHECK(focusables.empty());

		RecordingPaint paint;
		paintTree(*root, paint);
		CHECK(paint.fills == 0);
	}

	TEST_CASE("open dialog paints scrim and centers the card", "[Ui][Dialog]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Dialog dialog(*root, typeface);
		dialog.cardSize = { 360.0f, 240.0f };
		dialog.setOpen(true);
		settle(*root);

		CHECK(root->children().back().get() == &dialog.overlay());

		RecordingPaint paint;
		dialog.paint(paint);
		CHECK(paint.fills == 1);

		CHECK(dialog.card().world.size.x == 360.0f);
		CHECK(dialog.card().world.size.y == 240.0f);
		CHECK(dialog.card().world.pos.x == 220.0f);
		CHECK(dialog.card().world.pos.y == 180.0f);
	}

	TEST_CASE("title sits at the top, actions at the bottom, content fills the rest", "[Ui][Dialog]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Dialog dialog(*root, typeface);
		dialog.cardSize = { 360.0f, 240.0f };
		dialog.setTitle("Confirm");
		Panel body(dialog.content().addChild());
		body.node().intrinsicSize = glm::vec2{ 80.0f, 20.0f };
		dialog.addAction("OK", []() {});
		dialog.setOpen(true);
		settle(*root);

		const auto& card = dialog.card().world;
		const auto& title = dialog.card().children()[0]->world;
		const auto& content = dialog.content().world;
		const auto& actions = dialog.card().children()[2]->world;
		CHECK(title.pos.y == card.pos.y + 36.0f);
		CHECK(actions.pos.y + actions.size.y == card.pos.y + card.size.y - 16.0f);
		CHECK(content.pos.y == title.pos.y + title.size.y + 8.0f);
		CHECK(content.pos.y + content.size.y + 8.0f == actions.pos.y);
		CHECK(content.size.y > body.node().world.size.y);
		CHECK(card.size.y == 240.0f);
	}

	TEST_CASE("tall content expands the card past the preferred size", "[Ui][Dialog]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Dialog dialog(*root, typeface);
		dialog.cardSize = { 360.0f, 240.0f };
		Panel body(dialog.content().addChild());
		body.node().intrinsicSize = glm::vec2{ 100.0f, 400.0f };
		dialog.setOpen(true);
		settle(*root);

		CHECK(dialog.card().world.size.y > 240.0f);
		CHECK(dialog.content().world.size.y >= 400.0f);
	}

	TEST_CASE("empty title and no actions still layout content", "[Ui][Dialog]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Dialog dialog(*root, typeface);
		dialog.setTitle("");
		Panel body(dialog.content().addChild());
		body.node().intrinsicSize = glm::vec2{ 80.0f, 20.0f };
		dialog.setOpen(true);
		settle(*root);

		CHECK(body.node().world.size.x == 80.0f);
		CHECK(body.node().world.size.y == 20.0f);
		CHECK(body.node().world.size.x > 0.0f);
	}

	TEST_CASE("addAction click runs the callback", "[Ui][Dialog]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Dialog dialog(*root, typeface);
		int clicks = 0;
		Button& action = dialog.addAction("OK", [&]()
		{
			++clicks;
		});
		dialog.setOpen(true);
		settle(*root);

		Pointer pointer;
		pointer.position = action.node().world.pos + action.node().world.size * 0.5f;
		pointer.primaryReleased = true;
		dispatch(*root, pointer);
		CHECK(clicks == 1);
	}

	TEST_CASE("scrim click closes when enabled", "[Ui][Dialog]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Dialog dialog(*root, typeface);
		dialog.closeOnScrim = true;
		dialog.setOpen(true);
		settle(*root);

		Pointer pointer;
		pointer.position = { 10.0f, 10.0f };
		pointer.primaryReleased = true;
		dispatch(*root, pointer);
		CHECK_FALSE(dialog.isOpen());
	}

	TEST_CASE("scrim click does not close when disabled", "[Ui][Dialog]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Dialog dialog(*root, typeface);
		dialog.closeOnScrim = false;
		dialog.setOpen(true);
		settle(*root);

		Pointer pointer;
		pointer.position = { 10.0f, 10.0f };
		pointer.primaryReleased = true;
		dispatch(*root, pointer);
		CHECK(dialog.isOpen());
	}

	TEST_CASE("escape closes when enabled and stays open when disabled", "[Ui][Dialog]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Dialog dialog(*root, typeface);
		dialog.closeOnEscape = true;
		dialog.setOpen(true);
		settle(*root);
		dispatchTextKey(*root, TextKey::Escape);
		CHECK_FALSE(dialog.isOpen());

		dialog.closeOnEscape = false;
		dialog.setOpen(true);
		settle(*root);
		dispatchTextKey(*root, TextKey::Escape);
		CHECK(dialog.isOpen());
	}

	TEST_CASE("close button visibility follows the flag", "[Ui][Dialog]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Dialog dialog(*root, typeface);
		dialog.closeButtonVisible = true;
		dialog.setOpen(true);
		settle(*root);

		std::vector<Widget*> withClose;
		collectFocusables(*root, withClose);
		const auto closeCount = std::count_if(
			withClose.begin(),
			withClose.end(),
			[](Widget* widget)
			{
				return dynamic_cast<Button*>(widget) != nullptr;
			});
		CHECK(closeCount == 1);

		dialog.closeButtonVisible = false;
		settle(*root);
		std::vector<Widget*> withoutClose;
		collectFocusables(*root, withoutClose);
		const auto hiddenCount = std::count_if(
			withoutClose.begin(),
			withoutClose.end(),
			[](Widget* widget)
			{
				return dynamic_cast<Button*>(widget) != nullptr;
			});
		CHECK(hiddenCount == 0);
	}

	TEST_CASE("open dialog traps Tab away from outer widgets", "[Ui][Dialog]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		auto& column = root->addChild();
		column.kind = Kind::Column;
		Button outer(column.addChild(), typeface);
		outer.setText("outer");

		Dialog dialog(*root, typeface);
		dialog.addAction("OK", []() {});
		dialog.setOpen(true);
		settle(*root);

		for (int i = 0; i < 6; ++i)
		{
			dispatchTextKey(*root, TextKey::Tab);
			CHECK_FALSE(outer.hasKeyboardFocus());
		}
	}
}
