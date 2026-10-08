#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <algorithm>
#include <memory>
#include <vector>

namespace hl::ui::test
{
	namespace
	{
		class ClipRecordingPaint : public IPaint
		{
		public:
			int fills = 0;
			std::vector<Box> clips;
			int clipDepth = 0;
			int maxClipDepth = 0;

			void fill(const Box&, glm::vec4) override
			{
				++fills;
			}

			void sprite(const Box&, glm::vec4, glm::vec3) override {}

			void glyphs(const std::vector<GlyphVertex>&, glm::vec2, glm::vec3) override {}

			void pushClip(const Box& worldBox) override
			{
				clips.push_back(worldBox);
				++clipDepth;
				maxClipDepth = std::max(maxClipDepth, clipDepth);
			}

			void popClip() override
			{
				--clipDepth;
			}
		};

		void addSizedChild(Node& column, glm::vec2 size)
		{
			auto& child = column.addChild();
			child.intrinsicSize = size;
		}
	}

	TEST_CASE("Clip column keeps explicit size and overflow worlds", "[Ui][Layout]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();

		auto& clip = root->addChild();
		clip.kind = Kind::Column;
		clip.clip = true;
		clip.setTopLeft({ 100.0f, 80.0f });
		addSizedChild(clip, { 100.0f, 40.0f });
		addSizedChild(clip, { 100.0f, 40.0f });
		addSizedChild(clip, { 100.0f, 40.0f });

		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		CHECK(clip.world.pos.x == 0.0f);
		CHECK(clip.world.pos.y == 0.0f);
		CHECK(clip.world.size.x == 100.0f);
		CHECK(clip.world.size.y == 80.0f);
		CHECK(clip.contentSize().y == 120.0f);
		CHECK(clip.maxScroll().y == 40.0f);

		CHECK(clip.children()[0]->world.pos.y == 0.0f);
		CHECK(clip.children()[1]->world.pos.y == 40.0f);
		CHECK(clip.children()[2]->world.pos.y == 80.0f);
		CHECK(clip.children()[2]->world.size.y == 40.0f);
	}

	TEST_CASE("Scroll offset shifts packed children", "[Ui][Layout]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();

		auto& clip = root->addChild();
		clip.kind = Kind::Column;
		clip.clip = true;
		clip.setTopLeft({ 100.0f, 80.0f });
		addSizedChild(clip, { 100.0f, 40.0f });
		addSizedChild(clip, { 100.0f, 40.0f });
		addSizedChild(clip, { 100.0f, 40.0f });

		clip.scrollOffset.y = 40.0f;
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		CHECK(clip.children()[0]->world.pos.y == -40.0f);
		CHECK(clip.children()[1]->world.pos.y == 0.0f);
		CHECK(clip.children()[2]->world.pos.y == 40.0f);
		CHECK(clip.children()[2]->world.pos.y + clip.children()[2]->world.size.y == 80.0f);
	}

	TEST_CASE("clampScroll limits offset to maxScroll", "[Ui][Layout]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();

		auto& clip = root->addChild();
		clip.kind = Kind::Column;
		clip.clip = true;
		clip.setTopLeft({ 100.0f, 80.0f });
		addSizedChild(clip, { 100.0f, 40.0f });
		addSizedChild(clip, { 100.0f, 40.0f });
		addSizedChild(clip, { 100.0f, 40.0f });

		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		clip.scrollOffset.y = 999.0f;
		clip.clampScroll();
		CHECK(clip.scrollOffset.y == 40.0f);
	}

	TEST_CASE("Nested clip scroll shifts inner world then grandchildren", "[Ui][Layout]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();

		auto& outer = root->addChild();
		outer.clip = true;
		outer.setTopLeft({ 100.0f, 80.0f });

		auto& inner = outer.addChild();
		inner.kind = Kind::Column;
		inner.clip = true;
		inner.setTopLeft({ 100.0f, 40.0f });
		addSizedChild(inner, { 100.0f, 40.0f });
		addSizedChild(inner, { 100.0f, 40.0f });

		auto& filler = outer.addChild();
		filler.setTopLeft({ 100.0f, 40.0f });
		filler.relative = { 0.0f, 80.0f };

		outer.scrollOffset.y = 20.0f;
		inner.scrollOffset.y = 10.0f;
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		CHECK(inner.world.pos.y == -20.0f);
		CHECK(inner.children()[0]->world.pos.y == -30.0f);
		CHECK(inner.children()[1]->world.pos.y == 10.0f);
	}

	TEST_CASE("Hit test misses overflowing child outside clip", "[Ui][Widget]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();

		Panel clip(root->addChild());
		clip.node().kind = Kind::Column;
		clip.node().clip = true;
		clip.node().setTopLeft({ 100.0f, 80.0f });

		Panel a(clip.node().addChild());
		a.node().intrinsicSize = { 100.0f, 40.0f };
		Panel b(clip.node().addChild());
		b.node().intrinsicSize = { 100.0f, 40.0f };
		Panel overflow(clip.node().addChild());
		overflow.node().intrinsicSize = { 100.0f, 40.0f };

		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		CHECK(hitTest(*root, { 50.0f, 20.0f }) == &a);
		CHECK(hitTest(*root, { 50.0f, 100.0f }) != &overflow);
		CHECK(hitTest(*root, { 50.0f, 100.0f }) == nullptr);
	}

	TEST_CASE("applyScroll hits deepest clip and ignores outside", "[Ui][Widget]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();

		auto& outer = root->addChild();
		outer.clip = true;
		outer.setTopLeft({ 100.0f, 80.0f });

		auto& inner = outer.addChild();
		inner.kind = Kind::Column;
		inner.clip = true;
		inner.setTopLeft({ 100.0f, 40.0f });
		addSizedChild(inner, { 100.0f, 40.0f });
		addSizedChild(inner, { 100.0f, 40.0f });

		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		CHECK(applyScroll(*root, { 50.0f, 20.0f }, { 0.0f, 10.0f }));
		CHECK(inner.scrollOffset.y == 10.0f);
		CHECK(outer.scrollOffset.y == 0.0f);

		CHECK_FALSE(applyScroll(*root, { 50.0f, 200.0f }, { 0.0f, 10.0f }));
		CHECK(inner.scrollOffset.y == 10.0f);
		CHECK(outer.scrollOffset.y == 0.0f);
	}

	TEST_CASE("paintTree pushes clip boxes and still paints overflow fills", "[Ui][Widget]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();

		Panel clip(root->addChild());
		clip.node().kind = Kind::Column;
		clip.node().clip = true;
		clip.node().setTopLeft({ 100.0f, 80.0f });

		Panel a(clip.node().addChild());
		a.node().intrinsicSize = { 100.0f, 40.0f };
		Panel b(clip.node().addChild());
		b.node().intrinsicSize = { 100.0f, 40.0f };
		Panel c(clip.node().addChild());
		c.node().intrinsicSize = { 100.0f, 40.0f };

		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		ClipRecordingPaint paint;
		paintTree(*root, paint);
		REQUIRE(paint.clips.size() == 1);
		CHECK(paint.clips[0].pos.y == 0.0f);
		CHECK(paint.clips[0].size.y == 80.0f);
		CHECK(paint.clipDepth == 0);
		CHECK(paint.fills == 4);
	}
}
