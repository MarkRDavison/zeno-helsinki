#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Image.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
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
			glm::vec4 lastUv{ 0.0f };
			glm::vec3 lastColor{ 0.0f };

			void fill(const Box&, glm::vec4) override
			{
				++fills;
			}

			void sprite(const Box&, glm::vec4 uvRect, glm::vec3 color) override
			{
				++sprites;
				lastUv = uvRect;
				lastColor = color;
			}

			void glyphs(const std::vector<GlyphVertex>&, glm::vec2, glm::vec3) override {}
		};
	}

	TEST_CASE("prepare sets intrinsicSize from size", "[Ui][Image]")
	{
		auto root = std::make_unique<Node>();
		Image image(root->addChild());
		image.size = { 48.0f, 24.0f };
		image.prepare();
		REQUIRE(image.node().intrinsicSize.has_value());
		CHECK(image.node().intrinsicSize->x == 48.0f);
		CHECK(image.node().intrinsicSize->y == 24.0f);
	}

	TEST_CASE("paint is one sprite and no fill", "[Ui][Image]")
	{
		auto root = std::make_unique<Node>();
		Image image(root->addChild());
		image.size = { 32.0f, 32.0f };
		image.uvRect = { 0.0f, 0.0f, 1.0f, 1.0f };
		image.color = { 1.0f, 0.5f, 0.0f };
		image.prepare();
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		RecordingPaint paint;
		image.paint(paint);
		CHECK(paint.sprites == 1);
		CHECK(paint.fills == 0);
		CHECK(paint.lastUv == image.uvRect);
		CHECK(paint.lastColor == image.color);
	}

	TEST_CASE("layout world size matches size", "[Ui][Image]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Image image(root->addChild());
		image.size = { 128.0f, 64.0f };
		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		CHECK(image.node().world.size.x == 128.0f);
		CHECK(image.node().world.size.y == 64.0f);
	}

	TEST_CASE("collectFocusables omits Image", "[Ui][Image]")
	{
		auto root = std::make_unique<Node>();
		Image image(root->addChild());
		std::vector<Widget*> focusables;
		collectFocusables(*root, focusables);
		CHECK(focusables.empty());
	}
}
