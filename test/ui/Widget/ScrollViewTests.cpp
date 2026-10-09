#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/ScrollView.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <memory>

namespace hl::ui::test
{
	namespace
	{
		void addSizedChild(Node& column, glm::vec2 size)
		{
			column.addChild().intrinsicSize = size;
		}
	}

	TEST_CASE("taller content keeps the viewport height and can scroll", "[Ui][ScrollView]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();
		ScrollView view(root->addChild());
		view.viewportSize = { 100.0f, 80.0f };
		addSizedChild(view.content(), { 100.0f, 40.0f });
		addSizedChild(view.content(), { 100.0f, 40.0f });
		addSizedChild(view.content(), { 100.0f, 40.0f });

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		CHECK(view.viewport().world.size.y == 80.0f);
		CHECK(view.viewport().maxScroll().y > 0.0f);
	}

	TEST_CASE("applyScroll over the view increases offset and clamps", "[Ui][ScrollView]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();
		ScrollView view(root->addChild());
		view.viewportSize = { 100.0f, 80.0f };
		addSizedChild(view.content(), { 100.0f, 40.0f });
		addSizedChild(view.content(), { 100.0f, 40.0f });
		addSizedChild(view.content(), { 100.0f, 40.0f });

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		const auto inside = view.viewport().world.pos + glm::vec2{ 8.0f, 8.0f };
		REQUIRE(applyScroll(*root, inside, { 0.0f, 20.0f }));
		CHECK(view.viewport().scrollOffset.y == 20.0f);

		applyScroll(*root, inside, { 0.0f, 999.0f });
		CHECK(view.viewport().scrollOffset.y == view.viewport().maxScroll().y);
	}

	TEST_CASE("overflow with scrollBars shows a bar and thumb drag scrolls", "[Ui][ScrollView]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();
		ScrollView view(root->addChild());
		view.viewportSize = { 100.0f, 80.0f };
		view.scrollBars = true;
		addSizedChild(view.content(), { 100.0f, 40.0f });
		addSizedChild(view.content(), { 100.0f, 40.0f });
		addSizedChild(view.content(), { 100.0f, 40.0f });

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		REQUIRE(view.barVisible());
		REQUIRE(view.node().children().size() == 2);

		const auto& bar = *view.node().children()[1];
		const auto onThumb = bar.world.pos + glm::vec2{ 4.0f, 8.0f };
		dispatch(*root, Pointer{ .position = onThumb, .primaryDown = true });
		const auto lower = bar.world.pos + glm::vec2{ 4.0f, bar.world.size.y - 8.0f };
		dispatch(*root, Pointer{ .position = lower, .primaryDown = true });
		CHECK(view.viewport().scrollOffset.y > 0.0f);
	}

	TEST_CASE("scrollBars false hides the bar and wheel still works", "[Ui][ScrollView]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();
		ScrollView view(root->addChild());
		view.viewportSize = { 100.0f, 80.0f };
		view.scrollBars = false;
		addSizedChild(view.content(), { 100.0f, 40.0f });
		addSizedChild(view.content(), { 100.0f, 40.0f });
		addSizedChild(view.content(), { 100.0f, 40.0f });

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		CHECK_FALSE(view.barVisible());

		const auto inside = view.viewport().world.pos + glm::vec2{ 8.0f, 8.0f };
		REQUIRE(applyScroll(*root, inside, { 0.0f, 20.0f }));
		CHECK(view.viewport().scrollOffset.y == 20.0f);
	}

	TEST_CASE("short content hides the bar and has no scroll", "[Ui][ScrollView]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();
		ScrollView view(root->addChild());
		view.viewportSize = { 100.0f, 80.0f };
		addSizedChild(view.content(), { 100.0f, 20.0f });

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		CHECK_FALSE(view.barVisible());
		CHECK(view.viewport().maxScroll().y == 0.0f);
	}

	TEST_CASE("collectFocusables omits ScrollView and the bar", "[Ui][ScrollView]")
	{
		auto root = std::make_unique<Node>();
		ScrollView view(root->addChild());
		view.viewportSize = { 100.0f, 80.0f };
		addSizedChild(view.content(), { 100.0f, 40.0f });
		addSizedChild(view.content(), { 100.0f, 40.0f });
		addSizedChild(view.content(), { 100.0f, 40.0f });

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		REQUIRE(view.barVisible());

		std::vector<Widget*> focusables;
		collectFocusables(*root, focusables);
		CHECK(focusables.empty());
	}
}
