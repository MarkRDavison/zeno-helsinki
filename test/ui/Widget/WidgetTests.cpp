#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/Checkbox.hpp>
#include <helsinki/Ui/IconRow.hpp>
#include <helsinki/Ui/Label.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Slider.hpp>
#include <helsinki/Ui/Toggle.hpp>
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
				std::string_view,
				unsigned,
				std::vector<GlyphVertex>& out) const override
			{
				out.clear();
				out.push_back(GlyphVertex{ { 0.0f, 0.0f }, { 0.0f, 0.0f } });
				out.push_back(GlyphVertex{ { 80.0f, 30.0f }, { 1.0f, 1.0f } });
				return { 80.0f, 30.0f };
			}
		};

		class RecordingPaint : public IPaint
		{
		public:
			int fills = 0;
			int sprites = 0;
			int glyphBatches = 0;
			glm::vec4 lastFillColor{ 0.0f };
			glm::vec3 lastGlyphColor{ 0.0f };

			void fill(const Box&, glm::vec4 color) override
			{
				++fills;
				lastFillColor = color;
			}

			void sprite(const Box&, glm::vec4, glm::vec3) override
			{
				++sprites;
			}

			void glyphs(const std::vector<GlyphVertex>&, glm::vec2, glm::vec3 color) override
			{
				++glyphBatches;
				lastGlyphColor = color;
			}
		};
	}

	TEST_CASE("Label sets intrinsic size from typeface", "[Ui][Widget]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Label label(root->addChild(), typeface);
		label.setText("Hello", 24);
		label.prepare();

		CHECK(label.node().intrinsicSize.has_value());
		CHECK(label.node().intrinsicSize->x == 80.0f);
		CHECK(label.node().intrinsicSize->y == 30.0f);

		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		RecordingPaint paint;
		label.paint(paint);
		CHECK(paint.glyphBatches == 1);
	}

	TEST_CASE("Button click fires on hover and primaryReleased", "[Ui][Widget]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		auto& column = root->addChild();
		column.kind = Kind::Column;
		column.setCenter({ 0.0f, 0.0f });

		Button button(column.addChild(), typeface);
		button.setText("Start", 24);
		int clicks = 0;
		button.onClick = [&] { ++clicks; };
		button.prepare();
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		const auto center = button.node().world.pos + button.node().world.size * 0.5f;
		button.handle(Pointer{ .position = center, .primaryReleased = true });
		CHECK(clicks == 1);

		RecordingPaint paint;
		button.paint(paint);
		CHECK(paint.lastGlyphColor.y == 1.0f);
		CHECK(paint.lastGlyphColor.z == 0.0f);

		button.handle(Pointer{ .position = { 0.0f, 0.0f }, .primaryReleased = true });
		CHECK(clicks == 1);
	}

	TEST_CASE("Panel fills world box", "[Ui][Widget]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Panel panel(root->addChild());
		panel.node().setFillParent();
		panel.color = { 0.2f, 0.3f, 0.4f };
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		RecordingPaint paint;
		panel.paint(paint);
		CHECK(paint.fills == 1);
		CHECK(paint.lastFillColor.x == 0.2f);
	}

	class Probe : public Widget
	{
	public:
		using Widget::Widget;

		int handles = 0;
		EventResult result = EventResult::Ignore;

		EventResult handle(const Pointer&) override
		{
			++handles;
			return result;
		}
	};

	TEST_CASE("Label skips hit test; button consumes before panel", "[Ui][Widget]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();

		Probe panel(root->addChild());
		panel.node().setFillParent();

		Probe button(panel.node().addChild());
		button.node().setCenter({ 80.0f, 30.0f });
		button.result = EventResult::Consume;

		Label text(button.node().addChild(), typeface);
		text.node().setFillParent();
		text.setText("Start", 24);
		text.prepare();

		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		const auto onText = text.node().world.pos + text.node().world.size * 0.5f;
		CHECK(hitTest(*root, onText) == &button);

		dispatch(*root, Pointer{ .position = onText, .primaryReleased = true });
		CHECK(button.handles == 1);
		CHECK(panel.handles == 0);

		dispatch(*root, Pointer{ .position = { 8.0f, 8.0f }, .primaryReleased = true });
		CHECK(button.handles == 1);
		CHECK(panel.handles == 1);
		CHECK(hitTest(*root, { 8.0f, 8.0f }) == &panel);
	}

	TEST_CASE("Later sibling wins hit test", "[Ui][Widget]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();

		Probe first(root->addChild());
		first.node().setTopLeft({ 100.0f, 100.0f });
		first.result = EventResult::Consume;

		Probe second(root->addChild());
		second.node().setTopLeft({ 100.0f, 100.0f });
		second.result = EventResult::Consume;

		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		dispatch(*root, Pointer{ .position = { 50.0f, 50.0f }, .primaryReleased = true });
		CHECK(second.handles == 1);
		CHECK(first.handles == 0);
	}

	TEST_CASE("Ignore keeps bubbling to parent", "[Ui][Widget]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();

		Probe parent(root->addChild());
		parent.node().setFillParent();
		parent.result = EventResult::Consume;

		Probe child(parent.node().addChild());
		child.node().setFillParent();
		child.result = EventResult::Ignore;

		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		dispatch(*root, Pointer{ .position = { 10.0f, 10.0f }, .primaryReleased = false });
		CHECK(child.handles == 1);
		CHECK(parent.handles == 1);
	}

	TEST_CASE("Slider maps x to value and keeps capture while down", "[Ui][Widget]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Slider slider(root->addChild());
		slider.prepare();
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		const auto box = slider.node().world;
		dispatch(*root, Pointer{
			.position = { box.pos.x, box.pos.y + box.size.y * 0.5f },
			.primaryDown = true });
		CHECK(slider.value() < 0.05f);
		CHECK(slider.hasPointerCapture());

		dispatch(*root, Pointer{
			.position = { box.pos.x + box.size.x, box.pos.y + 200.0f },
			.primaryDown = true });
		CHECK(slider.value() > 0.95f);

		dispatch(*root, Pointer{
			.position = { box.pos.x + box.size.x, box.pos.y + 200.0f },
			.primaryDown = false });
		CHECK_FALSE(slider.hasPointerCapture());
	}

	TEST_CASE("Checkbox toggles on primaryReleased", "[Ui][Widget]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Checkbox box(root->addChild());
		box.prepare();
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		int changes = 0;
		box.onChanged = [&](bool) { ++changes; };
		const auto center = box.node().world.pos + box.node().world.size * 0.5f;
		dispatch(*root, Pointer{ .position = center, .primaryReleased = true });
		CHECK(box.checked());
		CHECK(changes == 1);

		RecordingPaint paint;
		box.paint(paint);
		CHECK(paint.fills == 2);
	}

	TEST_CASE("Toggle flips knob side on click", "[Ui][Widget]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Toggle toggle(root->addChild());
		toggle.prepare();
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		const auto center = toggle.node().world.pos + toggle.node().world.size * 0.5f;
		dispatch(*root, Pointer{ .position = center, .primaryReleased = true });
		CHECK(toggle.isOn());

		RecordingPaint paint;
		toggle.paint(paint);
		CHECK(paint.fills == 2);
	}

	TEST_CASE("IconRow sizes from count and paints one sprite per icon", "[Ui][Widget]")
	{
		auto root = std::make_unique<Node>();
		root->setFillParent();
		IconRow row(root->addChild());
		row.iconSize = { 32.0f, 32.0f };
		row.gap = 8.0f;
		row.setCount(3);
		row.prepare();

		CHECK(row.node().intrinsicSize.has_value());
		CHECK(row.node().intrinsicSize->x == 112.0f);
		CHECK(row.node().intrinsicSize->y == 32.0f);

		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		RecordingPaint paint;
		row.paint(paint);
		CHECK(paint.sprites == 3);

		row.setCount(0);
		row.prepare();
		CHECK(row.node().intrinsicSize->x == 0.0f);
		RecordingPaint empty;
		row.paint(empty);
		CHECK(empty.sprites == 0);
	}

	TEST_CASE("prepareTree and paintTree visit nested widgets", "[Ui][Widget]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		Panel panel(root->addChild());
		Label label(panel.node().addChild(), typeface);
		label.setText("Hi", 16);

		prepareTree(*root);
		layout(*root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });

		RecordingPaint paint;
		paintTree(*root, paint);
		CHECK(paint.fills == 1);
		CHECK(paint.glyphBatches == 1);
	}
}
