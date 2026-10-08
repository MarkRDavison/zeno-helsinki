#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/TextField.hpp>
#include <helsinki/Ui/Widget.hpp>

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
				const float width = static_cast<float>(text.size()) * 10.0f;
				if (width > 0.0f)
				{
					out.push_back(GlyphVertex{ { 0.0f, 0.0f }, { 0.0f, 0.0f } });
					out.push_back(GlyphVertex{ { width, 20.0f }, { 1.0f, 1.0f } });
				}

				return { width, 20.0f };
			}
		};

		class RecordingPaint : public IPaint
		{
		public:
			int fills = 0;
			int glyphBatches = 0;

			void fill(const Box&, glm::vec4) override
			{
				++fills;
			}

			void sprite(const Box&, glm::vec4, glm::vec3) override {}

			void glyphs(const std::vector<GlyphVertex>&, glm::vec2, glm::vec3) override
			{
				++glyphBatches;
			}
		};
	}

	TEST_CASE("TextField clips and focuses on click", "[Ui][Widget]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		TextField field(root->addChild(), typeface);
		field.node().setTopLeft({ 280.0f, 36.0f });
		CHECK(field.node().clip);
		field.prepare();
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		const auto inside = field.node().world.pos + field.node().world.size * 0.5f;
		dispatch(*root, Pointer{ .position = inside, .primaryReleased = true });
		CHECK(field.focused());
		dispatchChar(static_cast<uint32_t>('A'));
		CHECK(field.text() == "A");
	}

	TEST_CASE("Clicking a sibling blurs the TextField", "[Ui][Widget]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Panel panel(root->addChild());
		panel.node().setFillParent();
		TextField field(root->addChild(), typeface);
		field.node().setTopLeft({ 280.0f, 36.0f });
		field.prepare();
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		const auto onField = field.node().world.pos + glm::vec2{ 8.0f, 8.0f };
		dispatch(*root, Pointer{ .position = onField, .primaryReleased = true });
		dispatchChar(static_cast<uint32_t>('A'));
		CHECK(field.text() == "A");

		dispatch(*root, Pointer{ .position = { 400.0f, 400.0f }, .primaryReleased = true });
		CHECK_FALSE(field.focused());
		dispatchChar(static_cast<uint32_t>('B'));
		CHECK(field.text() == "A");
	}

	TEST_CASE("TextField inserts in the middle and backspaces", "[Ui][Widget]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		TextField field(root->addChild(), typeface);
		field.setFocused(true);
		dispatchChar(static_cast<uint32_t>('A'));
		dispatchChar(static_cast<uint32_t>('C'));
		dispatchTextKey(TextKey::Left);
		dispatchChar(static_cast<uint32_t>('B'));
		CHECK(field.text() == "ABC");

		dispatchTextKey(TextKey::End);
		dispatchTextKey(TextKey::Backspace);
		dispatchTextKey(TextKey::Backspace);
		CHECK(field.text() == "A");
	}

	TEST_CASE("Enter does not insert a newline", "[Ui][Widget]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		TextField field(root->addChild(), typeface);
		field.setFocused(true);
		dispatchChar(static_cast<uint32_t>('A'));
		dispatchTextKey(TextKey::Enter);
		CHECK(field.text() == "A");
	}

	TEST_CASE("paintTree still issues overflow glyphs", "[Ui][Widget]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		TextField field(root->addChild(), typeface);
		field.setFocused(true);
		field.setText("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH");
		field.prepare();
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		RecordingPaint paint;
		paintTree(*root, paint);
		CHECK(paint.fills >= 1);
		CHECK(paint.glyphBatches == 1);
	}
}
