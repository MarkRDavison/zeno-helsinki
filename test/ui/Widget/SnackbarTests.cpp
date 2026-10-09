#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Snackbar.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <cmath>
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
				out.push_back(GlyphVertex{ { 80.0f, 20.0f }, { 1.0f, 1.0f } });
				return { 80.0f, 20.0f };
			}
		};

		class RecordingPaint : public IPaint
		{
		public:
			std::vector<glm::vec4> fillColors;

			void fill(const Box&, glm::vec4 color) override
			{
				fillColors.push_back(color);
			}

			void sprite(const Box&, glm::vec4, glm::vec3) override {}
			void glyphs(const std::vector<GlyphVertex>&, glm::vec2, glm::vec3) override {}
		};

		void settle(Node& root)
		{
			prepareTree(root);
			layout(root, Box{ 0.0f, 0.0f, 800.0f, 600.0f });
		}

		SnackbarItem makeItem(SnackbarType type, std::string title, bool persistent = false)
		{
			return SnackbarItem{
				.type = type,
				.title = std::move(title),
				.description = "detail",
				.persistent = persistent,
				.duration = 1.0f,
				.fade = 0.5f
			};
		}
	}

	TEST_CASE("Snackbar show paints the type fill", "[Ui][Snackbar]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		SnackbarHost host(*root, typeface);
		host.show(makeItem(SnackbarType::Success, "ok"));
		settle(*root);

		CHECK(host.visibleCount() == 1);
		RecordingPaint paint;
		paintTree(*root, paint);
		REQUIRE(paint.fillColors.size() >= 2);
		CHECK(glm::vec3{ paint.fillColors[0] } == host.typeColor(SnackbarType::Success));
		CHECK(glm::vec3{ paint.fillColors[1] } == host.typeFill(SnackbarType::Success));
		CHECK(host.typeColor(SnackbarType::Success) != host.typeFill(SnackbarType::Success));
		REQUIRE(paint.fillColors.size() >= 3);
		CHECK(glm::vec3{ paint.fillColors[2] } == host.typeColor(SnackbarType::Success));
	}

	TEST_CASE("Snackbar timer shrinks from the right until fade", "[Ui][Snackbar]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		SnackbarHost host(*root, typeface);
		host.show(makeItem(SnackbarType::Info, "temp"));
		settle(*root);

		const float inner = theme().controlWidth - theme().padding * 2.0f;
		CHECK(std::abs(host.toastTimerWidth(0) - inner) < 0.5f);
		const float left = host.toastNode(0).world.pos.x + theme().padding;

		host.tick(0.5f);
		settle(*root);
		CHECK(std::abs(host.toastTimerWidth(0) - inner * 0.5f) < 0.5f);
		REQUIRE(host.toastNode(0).children().size() >= 3);
		CHECK(std::abs(host.toastNode(0).children()[2]->world.pos.x - left) < 0.5f);
	}

	TEST_CASE("Snackbar persistent sits at top center", "[Ui][Snackbar]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		SnackbarHost host(*root, typeface);
		host.corner = SnackbarCorner::BottomRight;
		host.show(makeItem(SnackbarType::Info, "corner"));
		host.show(makeItem(SnackbarType::Success, "stay", true));
		settle(*root);

		REQUIRE(host.visibleCount() == 2);
		const auto& transient = host.toastNode(0).world;
		const auto& persistent = host.toastNode(1).world;
		CHECK(persistent.pos.y < transient.pos.y);
		CHECK(std::abs(persistent.pos.x + persistent.size.x * 0.5f - 400.0f) < 1.0f);
		CHECK(persistent.pos.y == theme().paddingLarge);
		CHECK(host.toastTimerWidth(1) == 0.0f);
	}

	TEST_CASE("Snackbar newest occupies the corner and older shifts away", "[Ui][Snackbar]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		SnackbarHost host(*root, typeface);
		host.corner = SnackbarCorner::BottomRight;
		host.show(makeItem(SnackbarType::Info, "first"));
		host.show(makeItem(SnackbarType::Info, "second"));
		settle(*root);

		REQUIRE(host.visibleCount() == 2);
		const auto& older = host.toastNode(0).world;
		const auto& newer = host.toastNode(1).world;
		CHECK(newer.pos.y > older.pos.y);
		CHECK(newer.pos.x == older.pos.x);
		CHECK(newer.pos.y + newer.size.y <= 600.0f - theme().paddingLarge + 0.01f);
	}

	TEST_CASE("Snackbar drops oldest non-persistent at maxVisible", "[Ui][Snackbar]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		SnackbarHost host(*root, typeface);
		host.maxVisible = 2;
		host.show(makeItem(SnackbarType::Info, "a"));
		host.show(makeItem(SnackbarType::Warning, "b"));
		host.show(makeItem(SnackbarType::Error, "c"));
		settle(*root);

		REQUIRE(host.visibleCount() == 2);
		CHECK(host.queuedCount() == 0);
		CHECK(host.toastType(0) == SnackbarType::Warning);
		CHECK(host.toastType(1) == SnackbarType::Error);
	}

	TEST_CASE("Snackbar queues when every visible toast is persistent", "[Ui][Snackbar]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		SnackbarHost host(*root, typeface);
		host.maxVisible = 1;
		host.show(makeItem(SnackbarType::Info, "keep", true));
		host.show(makeItem(SnackbarType::Error, "wait"));
		settle(*root);

		REQUIRE(host.visibleCount() == 1);
		CHECK(host.toastPersistent(0));
		CHECK(host.queuedCount() == 1);

		host.requestClose(0);
		host.tick(0.5f);
		settle(*root);
		CHECK(host.visibleCount() == 1);
		CHECK_FALSE(host.toastPersistent(0));
		CHECK(host.toastType(0) == SnackbarType::Error);
		CHECK(host.queuedCount() == 0);
	}

	TEST_CASE("Snackbar fades after duration then removes", "[Ui][Snackbar]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		SnackbarHost host(*root, typeface);
		host.show(makeItem(SnackbarType::Info, "temp"));
		settle(*root);

		host.tick(1.0f);
		host.tick(0.1f);
		settle(*root);
		CHECK(host.visibleCount() == 1);
		CHECK(host.toastOpacity(0) < 1.0f);

		host.tick(0.4f);
		settle(*root);
		CHECK(host.visibleCount() == 0);
	}

	TEST_CASE("Snackbar persistent ignores duration until close", "[Ui][Snackbar]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		SnackbarHost host(*root, typeface);
		host.show(makeItem(SnackbarType::Warning, "stay", true));
		settle(*root);

		host.tick(8.0f);
		settle(*root);
		REQUIRE(host.visibleCount() == 1);
		CHECK(host.toastOpacity(0) == 1.0f);

		host.requestClose(0);
		host.tick(0.25f);
		settle(*root);
		CHECK(host.visibleCount() == 1);
		CHECK(host.toastOpacity(0) < 1.0f);

		host.tick(0.25f);
		settle(*root);
		CHECK(host.visibleCount() == 0);
	}

	TEST_CASE("Snackbar close button starts fade", "[Ui][Snackbar]")
	{
		FakeTypeface typeface;
		auto root = std::make_unique<Node>();
		root->setFillParent();
		SnackbarHost host(*root, typeface);
		host.show(makeItem(SnackbarType::Error, "close me"));
		settle(*root);

		const auto& toast = host.toastNode(0);
		REQUIRE_FALSE(toast.children().empty());
		const auto& closeBox = toast.children().back()->world;
		dispatch(*root, Pointer{
			.position = closeBox.pos + closeBox.size * 0.5f,
			.primaryReleased = true
		});
		host.tick(0.01f);
		settle(*root);
		CHECK(host.visibleCount() == 1);
		CHECK(host.toastOpacity(0) < 1.0f);
	}
}
