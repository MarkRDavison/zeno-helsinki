#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>

#include <memory>

namespace hl::ui::test
{
	namespace
	{
		const Box kViewport{ 0.0f, 0.0f, 800.0f, 600.0f };

		void checkVec2(glm::vec2 actual, glm::vec2 expected)
		{
			CHECK(actual.x == expected.x);
			CHECK(actual.y == expected.y);
		}

		void checkBox(const Box& actual, glm::vec2 pos, glm::vec2 size)
		{
			checkVec2(actual.pos, pos);
			checkVec2(actual.size, size);
		}

		std::unique_ptr<Node> makeFillRoot()
		{
			auto root = std::make_unique<Node>();
			root->setFillParent();
			return root;
		}
	}

	TEST_CASE("Fill parent", "[Ui][Layout]")
	{
		auto root = makeFillRoot();
		layout(*root, kViewport);
		checkBox(root->world, { 0.0f, 0.0f }, { 800.0f, 600.0f });
	}

	TEST_CASE("Point anchors nine presets", "[Ui][Layout]")
	{
		const glm::vec2 size{ 100.0f, 50.0f };

		auto checkPreset = [&](auto setter, glm::vec2 expectedPos)
		{
			auto root = makeFillRoot();
			Node& child = root->addChild();
			setter(child, size);
			layout(*root, kViewport);
			checkBox(child.world, expectedPos, size);
		};

		checkPreset([](Node& n, glm::vec2 s) { n.setTopLeft(s); }, { 0.0f, 0.0f });
		checkPreset([](Node& n, glm::vec2 s) { n.setTopCenter(s); }, { 350.0f, 0.0f });
		checkPreset([](Node& n, glm::vec2 s) { n.setTopRight(s); }, { 700.0f, 0.0f });
		checkPreset([](Node& n, glm::vec2 s) { n.setCenterLeft(s); }, { 0.0f, 275.0f });
		checkPreset([](Node& n, glm::vec2 s) { n.setCenter(s); }, { 350.0f, 275.0f });
		checkPreset([](Node& n, glm::vec2 s) { n.setCenterRight(s); }, { 700.0f, 275.0f });
		checkPreset([](Node& n, glm::vec2 s) { n.setBottomLeft(s); }, { 0.0f, 550.0f });
		checkPreset([](Node& n, glm::vec2 s) { n.setBottomCenter(s); }, { 350.0f, 550.0f });
		checkPreset([](Node& n, glm::vec2 s) { n.setBottomRight(s); }, { 700.0f, 550.0f });
	}

	TEST_CASE("Point anchor relative offset", "[Ui][Layout]")
	{
		auto root = makeFillRoot();
		Node& child = root->addChild();
		child.setTopLeft({ 100.0f, 50.0f });
		child.relative = { 10.0f, 20.0f };
		layout(*root, kViewport);
		checkBox(child.world, { 10.0f, 20.0f }, { 100.0f, 50.0f });
	}

	TEST_CASE("Point anchor offset edges", "[Ui][Layout]")
	{
		auto root = makeFillRoot();
		Node& child = root->addChild();
		child.setTopLeft({ 100.0f, 50.0f });
		child.offset.left = 10.0f;
		child.offset.top = 20.0f;
		child.offset.right = 110.0f;
		child.offset.bottom = 70.0f;
		layout(*root, kViewport);
		checkBox(child.world, { 10.0f, 20.0f }, { 100.0f, 50.0f });
	}

	TEST_CASE("Stretch horizontal", "[Ui][Layout]")
	{
		auto root = makeFillRoot();
		Node& child = root->addChild();
		child.anchorMin = { 0.0f, 0.0f };
		child.anchorMax = { 1.0f, 0.0f };
		child.offset.left = 16.0f;
		child.offset.right = -16.0f;
		child.offset.top = 16.0f;
		child.offset.bottom = 64.0f;
		layout(*root, kViewport);
		checkBox(child.world, { 16.0f, 16.0f }, { 768.0f, 48.0f });
	}

	TEST_CASE("Stretch vertical", "[Ui][Layout]")
	{
		auto root = makeFillRoot();
		Node& child = root->addChild();
		child.anchorMin = { 0.0f, 0.0f };
		child.anchorMax = { 0.0f, 1.0f };
		child.offset.left = 16.0f;
		child.offset.right = 144.0f;
		child.offset.top = 16.0f;
		child.offset.bottom = -16.0f;
		layout(*root, kViewport);
		checkBox(child.world, { 16.0f, 16.0f }, { 128.0f, 568.0f });
	}

	TEST_CASE("Nested parent", "[Ui][Layout]")
	{
		auto root = makeFillRoot();
		Node& mid = root->addChild();
		mid.setTopLeft({ 200.0f, 100.0f });
		mid.relative = { 10.0f, 20.0f };
		Node& child = mid.addChild();
		child.setTopLeft({ 50.0f, 40.0f });
		child.relative = { 5.0f, 6.0f };

		layout(*root, kViewport);

		checkBox(mid.world, { 10.0f, 20.0f }, { 200.0f, 100.0f });
		checkBox(child.local, { 5.0f, 6.0f }, { 50.0f, 40.0f });
		checkBox(child.world, { 15.0f, 26.0f }, { 50.0f, 40.0f });
	}

	TEST_CASE("Relative on center", "[Ui][Layout]")
	{
		auto root = makeFillRoot();
		Node& child = root->addChild();
		child.setCenter({ 100.0f, 50.0f });
		child.relative = { 10.0f, -4.0f };
		layout(*root, kViewport);
		checkBox(child.world, { 360.0f, 271.0f }, { 100.0f, 50.0f });
	}

	TEST_CASE("Intrinsic size wins on pinned axis", "[Ui][Layout]")
	{
		auto root = makeFillRoot();
		Node& child = root->addChild();
		child.setCenter({ 10.0f, 10.0f });
		child.intrinsicSize = glm::vec2{ 80.0f, 30.0f };
		layout(*root, kViewport);
		checkBox(child.world, { 360.0f, 285.0f }, { 80.0f, 30.0f });
	}

	TEST_CASE("Column pack start", "[Ui][Layout]")
	{
		auto root = makeFillRoot();
		Node& column = root->addChild();
		column.kind = Kind::Column;
		column.gap = 10.0f;
		column.padding = {};
		column.crossAlign = Align::Start;
		column.setCenter({ 0.0f, 0.0f });

		Node& a = column.addChild();
		a.intrinsicSize = glm::vec2{ 100.0f, 20.0f };
		Node& b = column.addChild();
		b.intrinsicSize = glm::vec2{ 80.0f, 20.0f };

		layout(*root, kViewport);

		checkBox(column.world, { 350.0f, 275.0f }, { 100.0f, 50.0f });
		checkBox(a.world, { 350.0f, 275.0f }, { 100.0f, 20.0f });
		checkBox(b.world, { 350.0f, 305.0f }, { 80.0f, 20.0f });
	}

	TEST_CASE("Column crossAlign center", "[Ui][Layout]")
	{
		auto root = makeFillRoot();
		Node& column = root->addChild();
		column.kind = Kind::Column;
		column.gap = 10.0f;
		column.crossAlign = Align::Center;
		column.setCenter({ 0.0f, 0.0f });

		Node& a = column.addChild();
		a.intrinsicSize = glm::vec2{ 100.0f, 20.0f };
		Node& b = column.addChild();
		b.intrinsicSize = glm::vec2{ 80.0f, 20.0f };

		layout(*root, kViewport);

		checkBox(column.world, { 350.0f, 275.0f }, { 100.0f, 50.0f });
		CHECK(b.world.pos.x == 360.0f);
	}

	TEST_CASE("Column padding", "[Ui][Layout]")
	{
		auto root = makeFillRoot();
		Node& column = root->addChild();
		column.kind = Kind::Column;
		column.gap = 10.0f;
		column.padding = Edges::all(8.0f);
		column.crossAlign = Align::Start;
		column.setCenter({ 0.0f, 0.0f });

		Node& a = column.addChild();
		a.intrinsicSize = glm::vec2{ 100.0f, 20.0f };
		Node& b = column.addChild();
		b.intrinsicSize = glm::vec2{ 80.0f, 20.0f };

		layout(*root, kViewport);

		checkBox(column.world, { 342.0f, 267.0f }, { 116.0f, 66.0f });
		checkBox(a.world, { 350.0f, 275.0f }, { 100.0f, 20.0f });
		checkBox(b.world, { 350.0f, 305.0f }, { 80.0f, 20.0f });
	}

	TEST_CASE("Empty column padding", "[Ui][Layout]")
	{
		auto root = makeFillRoot();
		Node& column = root->addChild();
		column.kind = Kind::Column;
		column.padding = Edges::all(8.0f);
		column.setCenter({ 0.0f, 0.0f });
		layout(*root, kViewport);
		checkBox(column.world, { 392.0f, 292.0f }, { 16.0f, 16.0f });
	}

	TEST_CASE("Row", "[Ui][Layout]")
	{
		auto root = makeFillRoot();
		Node& row = root->addChild();
		row.kind = Kind::Row;
		row.gap = 4.0f;
		row.setCenter({ 0.0f, 0.0f });

		Node& a = row.addChild();
		a.intrinsicSize = glm::vec2{ 20.0f, 40.0f };
		Node& b = row.addChild();
		b.intrinsicSize = glm::vec2{ 30.0f, 40.0f };

		layout(*root, kViewport);

		checkBox(row.world, { 373.0f, 280.0f }, { 54.0f, 40.0f });
		checkBox(a.world, { 373.0f, 280.0f }, { 20.0f, 40.0f });
		checkBox(b.world, { 397.0f, 280.0f }, { 30.0f, 40.0f });
	}

	TEST_CASE("Resize same tree", "[Ui][Layout]")
	{
		auto root = makeFillRoot();
		Node& child = root->addChild();
		child.setCenter({ 100.0f, 50.0f });

		layout(*root, kViewport);
		checkBox(child.world, { 350.0f, 275.0f }, { 100.0f, 50.0f });

		layout(*root, Box{ 0.0f, 0.0f, 400.0f, 300.0f });
		checkBox(child.world, { 150.0f, 125.0f }, { 100.0f, 50.0f });
	}

	TEST_CASE("Mix anchors and container ignores child anchors", "[Ui][Layout]")
	{
		auto root = makeFillRoot();
		Node& column = root->addChild();
		column.kind = Kind::Column;
		column.gap = 10.0f;
		column.crossAlign = Align::Start;
		column.setCenter({ 0.0f, 0.0f });

		Node& a = column.addChild();
		a.intrinsicSize = glm::vec2{ 100.0f, 20.0f };
		a.setCenter({ 40.0f, 40.0f });
		Node& b = column.addChild();
		b.intrinsicSize = glm::vec2{ 80.0f, 20.0f };
		b.setCenter({ 40.0f, 40.0f });

		layout(*root, kViewport);

		checkBox(column.world, { 350.0f, 275.0f }, { 100.0f, 50.0f });
		checkBox(a.world, { 350.0f, 275.0f }, { 100.0f, 20.0f });
		checkBox(b.world, { 350.0f, 305.0f }, { 80.0f, 20.0f });
	}
}
