#include <catch2/catch_test_macros.hpp>
#include <helsinki/Ui/Label.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Theme.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <memory>

namespace hl::ui::test
{
	namespace
	{
		class RecordingTypeface : public ITypeface
		{
		public:
			mutable unsigned lastSize = 0;

			glm::vec2 layoutText(
				std::string_view,
				unsigned fontSize,
				std::vector<GlyphVertex>& out) const override
			{
				lastSize = fontSize;
				out.clear();
				return { 8.0f, 8.0f };
			}
		};

		struct ThemeFontGuard
		{
			unsigned previous;

			explicit ThemeFontGuard(unsigned fontSize) :
				previous(theme().fontSize)
			{
				theme().fontSize = fontSize;
			}

			~ThemeFontGuard()
			{
				theme().fontSize = previous;
			}
		};
	}

	TEST_CASE("unoverridden widgets use theme font size", "[Ui][Widget][Theme]")
	{
		ThemeFontGuard guard(11);
		RecordingTypeface typeface;
		auto root = std::make_unique<Node>();
		Label label(root->addChild(), typeface);
		label.setText("theme");
		label.prepare();
		CHECK(label.resolvedFontSize() == 11);
		CHECK(typeface.lastSize == 11);
	}

	TEST_CASE("widget fontSize overrides theme", "[Ui][Widget][Theme]")
	{
		ThemeFontGuard guard(11);
		RecordingTypeface typeface;
		auto root = std::make_unique<Node>();
		Label label(root->addChild(), typeface);
		label.setText("local", 24);
		label.prepare();
		CHECK(label.resolvedFontSize() == 24);
		CHECK(typeface.lastSize == 24);
		CHECK(theme().fontSize == 11);
	}
}
