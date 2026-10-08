#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <memory>

namespace hl::ui::test
{
	namespace
	{
		class RecordingPaint : public IPaint
		{
		public:
			int fills = 0;
			int sprites = 0;

			void fill(const Box&, glm::vec4) override
			{
				++fills;
			}

			void sprite(const Box&, glm::vec4, glm::vec3) override
			{
				++sprites;
			}

			void glyphs(const std::vector<GlyphVertex>&, glm::vec2, glm::vec3) override {}
		};
	}

	TEST_CASE("nineSlice emits nine sprites for a large dest", "[Ui][NineSlice]")
	{
		RecordingPaint paint;
		paint.nineSlice(
			Box{ 10.0f, 20.0f, 200.0f, 120.0f },
			NineSlice{});
		CHECK(paint.sprites == 9);
		CHECK(paint.fills == 0);
	}

	TEST_CASE("nineSlice skips empty patches when dest is smaller than borders", "[Ui][NineSlice]")
	{
		RecordingPaint paint;
		paint.nineSlice(
			Box{ 0.0f, 0.0f, 10.0f, 10.0f },
			NineSlice{ .slice = Edges::all(8.0f) });
		CHECK(paint.sprites < 9);
		CHECK(paint.sprites >= 1);
		CHECK(paint.fills == 0);
	}

	TEST_CASE("Panel borderWidth paints two fills", "[Ui][NineSlice]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Panel panel(root->addChild());
		panel.node().setFillParent();
		panel.borderWidth = 4.0f;
		panel.color = { 0.2f, 0.3f, 0.4f };
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		RecordingPaint paint;
		panel.paint(paint);
		CHECK(paint.fills == 2);
		CHECK(paint.sprites == 0);
	}

	TEST_CASE("Panel nineSlice paints sprites and no fills", "[Ui][NineSlice]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Panel panel(root->addChild());
		panel.node().setTopLeft({ 200.0f, 120.0f });
		panel.nineSlice = NineSlice{};
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		RecordingPaint paint;
		panel.paint(paint);
		CHECK(paint.sprites == 9);
		CHECK(paint.fills == 0);
	}
}
