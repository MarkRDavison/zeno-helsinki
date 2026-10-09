#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <algorithm>
#include <memory>

namespace hl::ui::test
{
namespace ButtonTests
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

	TEST_CASE("enabled Button click fires onClick", "[Ui][Button]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Button button(root->addChild(), typeface);
		button.setText("go", 16);
		int clicks = 0;
		button.onClick = [&clicks]() { ++clicks; };
		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		const auto onButton = button.node().world.pos + glm::vec2{ 8.0f, 8.0f };
		dispatch(*root, Pointer{ .position = onButton, .primaryReleased = true });
		CHECK(clicks == 1);
	}

	TEST_CASE("disabled Button click does not fire onClick", "[Ui][Button]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Button button(root->addChild(), typeface);
		button.setText("go", 16);
		button.enabled = false;
		int clicks = 0;
		button.onClick = [&clicks]() { ++clicks; };
		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		const auto onButton = button.node().world.pos + glm::vec2{ 8.0f, 8.0f };
		dispatch(*root, Pointer{ .position = onButton, .primaryReleased = true });
		CHECK(clicks == 0);
	}

	TEST_CASE("disabled Button still hit-tests", "[Ui][Button]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Button button(root->addChild(), typeface);
		button.setText("go", 16);
		button.enabled = false;
		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		const auto onButton = button.node().world.pos + glm::vec2{ 8.0f, 8.0f };
		CHECK(hitTest(*root, onButton) == &button);
	}

	TEST_CASE("disabled Button Space and Enter do not fire onClick", "[Ui][Button]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Button button(root->addChild(), typeface);
		button.setText("go", 16);
		button.enabled = false;
		int clicks = 0;
		button.onClick = [&clicks]() { ++clicks; };
		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		button.setFocused(true);
		dispatchTextKey(*root, TextKey::Space);
		dispatchTextKey(*root, TextKey::Enter);
		CHECK(clicks == 0);
	}

}
}
