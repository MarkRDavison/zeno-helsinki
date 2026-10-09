#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/ProgressBar.hpp>
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

			void fill(const Box&, glm::vec4) override
			{
				++fills;
			}

			void sprite(const Box&, glm::vec4, glm::vec3) override {}
			void glyphs(const std::vector<GlyphVertex>&, glm::vec2, glm::vec3) override {}
		};
	}

	TEST_CASE("ProgressBar clamps value to 0-1", "[Ui][ProgressBar]")
	{
		auto root = std::make_unique<Node>();
		ProgressBar bar(root->addChild());
		bar.setValue(-0.5f);
		CHECK(bar.value() == 0.0f);
		bar.setValue(1.5f);
		CHECK(bar.value() == 1.0f);
		bar.setValue(0.25f);
		CHECK(bar.value() == 0.25f);
	}

	TEST_CASE("ProgressBar paint is track only at 0 and two fills at 0.5", "[Ui][ProgressBar]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();
		ProgressBar bar(root->addChild());
		bar.node().setTopLeft({ 280.0f, 16.0f });
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		RecordingPaint empty;
		bar.setValue(0.0f);
		bar.paint(empty);
		CHECK(empty.fills == 1);

		RecordingPaint half;
		bar.setValue(0.5f);
		bar.paint(half);
		CHECK(half.fills == 2);
	}

	TEST_CASE("ProgressBar is not focusable", "[Ui][ProgressBar]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();
		ProgressBar bar(root->addChild());
		std::vector<Widget*> focusables;
		collectFocusables(*root, focusables);
		CHECK(focusables.empty());
		CHECK_FALSE(bar.hitTestEnabled);
	}
}
