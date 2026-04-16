#include <catch2/catch_test_macros.hpp>
#include <fakeit.hpp>
#include <helsinki/UserInterface/Layout.hpp>
#include "../TestHelpers.hpp"

namespace hl
{
	namespace tests
	{

		TEST_CASE("UserInterface::Alignment::EAlignment single horizontal flags have distinct bits", "[layout][alignment]")
		{
			REQUIRE((Left & HCenter) == 0);
			REQUIRE((Left & Right) == 0);
			REQUIRE((HCenter & Right) == 0);
		}

        TEST_CASE("UserInterface::Alignment::EAlignment single vertical flags have distinct bits", "[layout][alignment]")
        {
            REQUIRE((Top & VCenter) == 0);
            REQUIRE((Top & Bottom) == 0);
            REQUIRE((VCenter & Bottom) == 0);
        }

        TEST_CASE("UserInterface::Alignment::EAlignment combined presets match their component flags", "[layout][alignment]")
        {
            REQUIRE(TopLeft == (Top | Left));
            REQUIRE(TopCenter == (Top | HCenter));
            REQUIRE(TopRight == (Top | Right));
            REQUIRE(CenterLeft == (VCenter | Left));
            REQUIRE(Center == (VCenter | HCenter));
            REQUIRE(CenterRight == (VCenter | Right));
            REQUIRE(BottomLeft == (Bottom | Left));
            REQUIRE(BottomCenter == (Bottom | HCenter));
            REQUIRE(BottomRight == (Bottom | Right));
        }

        TEST_CASE("UserInterface::Constraints::Unbounded has zero min and max float max", "[layout][constraints]")
        {
            Constraints c = Constraints::Unbounded();
            REQUIRE(c.MinSize[0] == 0.0f);
            REQUIRE(c.MinSize[1] == 0.0f);
            REQUIRE(c.MaxSize[0] == std::numeric_limits<f32>::max());
            REQUIRE(c.MaxSize[1] == std::numeric_limits<f32>::max());
        }

        TEST_CASE("UserInterface::Constraints::Fixed sets min and max to the same size", "[layout][constraints]")
        {
            Constraints c = Constraints::Fixed(Vec2f(100.0f, 200.0f));
            RequireApproxEqual(c.MinSize, Vec2f(100.0f, 200.0f));
            RequireApproxEqual(c.MaxSize, Vec2f(100.0f, 200.0f));
        }

        TEST_CASE("UserInterface::Constraints::AtLeast sets min and leaves max unbounded", "[layout][constraints]")
        {
            Constraints c = Constraints::AtLeast(Vec2f(50.0f, 75.0f));
            RequireApproxEqual(c.MinSize, Vec2f(50.0f, 75.0f));
            REQUIRE(c.MaxSize[0] == std::numeric_limits<f32>::max());
            REQUIRE(c.MaxSize[1] == std::numeric_limits<f32>::max());
        }

        TEST_CASE("UserInterface::Constraints::AtMost leaves min at zero and sets max", "[layout][constraints]")
        {
            Constraints c = Constraints::AtMost(Vec2f(300.0f, 400.0f));
            RequireApproxEqual(c.MinSize, Vec2f(0.0f, 0.0f));
            RequireApproxEqual(c.MaxSize, Vec2f(300.0f, 400.0f));
        }

        TEST_CASE("UserInterface::Visibility::Visibility defaults to Visible", "[layout][visibility]")
        {
            Visibility v{};
            REQUIRE(v.Value == Visibility::Visible);
        }

        TEST_CASE("UserInterface::Visibility::Visibility Visible AffectsLayout and IsRendered", "[layout][visibility]")
        {
            Visibility v{ Visibility::Visible };
            REQUIRE(v.AffectsLayout());
            REQUIRE(v.IsRendered());
        }

        TEST_CASE("UserInterface::Visibility::Visibility Hidden AffectsLayout but not IsRendered", "[layout][visibility]")
        {
            Visibility v{ Visibility::Hidden };
            REQUIRE(v.AffectsLayout());
            REQUIRE_FALSE(v.IsRendered());
        }

        TEST_CASE("UserInterface::Visibility::Visibility Collapsed does not affect layout and is not rendered", "[layout][visibility]")
        {
            Visibility v{ Visibility::Collapsed };
            REQUIRE_FALSE(v.AffectsLayout());
            REQUIRE_FALSE(v.IsRendered());
        }

        // TODO: Get Edges tests

        TEST_CASE("UserInterface::Anchor::Anchor default construction produces all-zero fields", "[layout][anchor]")
        {
            Anchor a{};
            RequireApproxEqual(a.Min, Vec2f(0.0f, 0.0f));
            RequireApproxEqual(a.Max, Vec2f(0.0f, 0.0f));
            RequireApproxEqual(a.Pivot, Vec2f(0.0f, 0.0f));
            RequireApproxEqual(a.Offset, Vec2f(0.0f, 0.0f));
        }

        TEST_CASE("UserInterface::Anchor::TopLeft has min and max at (0,0)", "[layout][anchor]")
        {
            Anchor a = Anchor::TopLeft();
            RequireApproxEqual(a.Min, Vec2f(0.0f, 0.0f));
            RequireApproxEqual(a.Max, Vec2f(0.0f, 0.0f));
            RequireApproxEqual(a.Pivot, Vec2f(0.0f, 0.0f));
        }

        TEST_CASE("UserInterface::Anchor::Center has min and max at (0.5,0.5)", "[layout][anchor]")
        {
            Anchor a = Anchor::Center();
            RequireApproxEqual(a.Min, Vec2f(0.5f, 0.5f));
            RequireApproxEqual(a.Max, Vec2f(0.5f, 0.5f));
            RequireApproxEqual(a.Pivot, Vec2f(0.5f, 0.5f));
        }

        TEST_CASE("UserInterface::Anchor::BottomRight has min and max at (1,1)", "[layout][anchor]")
        {
            Anchor a = Anchor::BottomRight();
            RequireApproxEqual(a.Min, Vec2f(1.0f, 1.0f));
            RequireApproxEqual(a.Max, Vec2f(1.0f, 1.0f));
            RequireApproxEqual(a.Pivot, Vec2f(1.0f, 1.0f));
        }

        TEST_CASE("UserInterface::Anchor::StretchAll spans the full parent", "[layout][anchor]")
        {
            Anchor a = Anchor::StretchAll();
            RequireApproxEqual(a.Min, Vec2f(0.0f, 0.0f));
            RequireApproxEqual(a.Max, Vec2f(1.0f, 1.0f));
            RequireApproxEqual(a.Pivot, Vec2f(0.5f, 0.5f));
        }

        TEST_CASE("UserInterface::Anchor::TopCenter has horizontal center anchor", "[layout][anchor]")
        {
            Anchor a = Anchor::TopCenter();
            REQUIRE(a.Min[0] == Catch::Approx(0.5f));
            REQUIRE(a.Min[1] == Catch::Approx(0.0f));
            REQUIRE(a.Max[0] == Catch::Approx(0.5f));
            REQUIRE(a.Max[1] == Catch::Approx(0.0f));
        }

        TEST_CASE("UserInterface::Anchor::StretchTop stretches horizontally along the top edge", "[layout][anchor]")
        {
            Anchor a = Anchor::StretchTop();
            RequireApproxEqual(a.Min, Vec2f(0.0f, 0.0f));
            RequireApproxEqual(a.Max, Vec2f(1.0f, 0.0f));
        }

        TEST_CASE("UserInterface::Anchor::StretchLeft stretches vertically along the left edge", "[layout][anchor]")
        {
            Anchor a = Anchor::StretchLeft();
            RequireApproxEqual(a.Min, Vec2f(0.0f, 0.0f));
            RequireApproxEqual(a.Max, Vec2f(0.0f, 1.0f));
        }

        TEST_CASE("UserInterface::Style::LayoutStyle defaults have zero spacing", "[layout][style]")
        {
            LayoutStyle s{};
            REQUIRE(s.Spacing == 0.0f);
        }

        TEST_CASE("UserInterface::Style::LayoutStyle defaults to Overlay layout type", "[layout][style]")
        {
            LayoutStyle s{};
            REQUIRE(s.LayoutType == ELayoutType::Overlay);
        }

        TEST_CASE("UserInterface::Style::LayoutStyle defaults to TopLeft child alignment", "[layout][style]")
        {
            LayoutStyle s{};
            REQUIRE(s.ChildAlign == EAlignment::TopLeft);
        }

        TEST_CASE("UserInterface::Style::LayoutStyle defaults to NoWrap wrap mode", "[layout][style]")
        {
            LayoutStyle s{};
            REQUIRE(s.WrapMode == EWrapMode::NoWrap);
        }

        TEST_CASE("UserInterface::Style::LayoutStyle defaults to Flow positioning mode", "[layout][style]")
        {
            LayoutStyle s{};
            REQUIRE(s.PositionMode == EPositioningMode::Flow);
        }

        TEST_CASE("UserInterface::Style::LayoutStyle defaults to Content sizing modes", "[layout][style]")
        {
            LayoutStyle s{};
            REQUIRE(s.WidthMode == ESizingMode::Content);
            REQUIRE(s.HeightMode == ESizingMode::Content);
        }

        TEST_CASE("UserInterface::Style::LayoutStyle defaults to Inherit self-alignment", "[layout][style]")
        {
            LayoutStyle s{};
            REQUIRE(s.SelfAlign == EAlignment::Inherit);
        }

        TEST_CASE("UserInterface::Style::LayoutStyle defaults to zero fixed sizes", "[layout][style]")
        {
            LayoutStyle s{};
            REQUIRE(s.FixedWidth == 0.0f);
            REQUIRE(s.FixedHeight == 0.0f);
        }

        TEST_CASE("UserInterface::Style::LayoutStyle defaults to zero flex grow", "[layout][style]")
        {
            LayoutStyle s{};
            REQUIRE(s.FlexGrow == 0.0f);
        }

        TEST_CASE("UserInterface::Style::LayoutStyle defaults have zero padding and margin", "[layout][style]")
        {
            LayoutStyle s{};
            REQUIRE(s.Padding.Top == 0.0f);
            REQUIRE(s.Padding.Right == 0.0f);
            REQUIRE(s.Padding.Bottom == 0.0f);
            REQUIRE(s.Padding.Left == 0.0f);
            REQUIRE(s.Margin.Top == 0.0f);
            REQUIRE(s.Margin.Right == 0.0f);
            REQUIRE(s.Margin.Bottom == 0.0f);
            REQUIRE(s.Margin.Left == 0.0f);
        }
        
        TEST_CASE("UserInterface::Style::LayoutStyle SizeConstraints defaults to Unbounded", "[layout][style]")
        {
            LayoutStyle s{};
            REQUIRE(s.SizeConstraints.MinSize[0] == 0.0f);
            REQUIRE(s.SizeConstraints.MinSize[1] == 0.0f);
            REQUIRE(s.SizeConstraints.MaxSize[0] == std::numeric_limits<f32>::max());
            REQUIRE(s.SizeConstraints.MaxSize[1] == std::numeric_limits<f32>::max());
        }

        TEST_CASE("UserInterface::Result::LayoutResult defaults to dirty", "[layout][result]")
        {
            LayoutResult r{};
            REQUIRE(r.IsDirty);
        }
        
        TEST_CASE("UserInterface::Result::LayoutResult defaults to zero desired size", "[layout][result]")
        {
            LayoutResult r{};
            RequireApproxEqual(r.DesiredSize, Vec2f(0.0f, 0.0f));
        }
        
        TEST_CASE("UserInterface::Result::LayoutResult defaults to zero final rect", "[layout][result]")
        {
            LayoutResult r{};
            RequireApproxEqual(r.FinalRect.Min(), Vec2f(0.0f, 0.0f));
            RequireApproxEqual(r.FinalRect.Max(), Vec2f(0.0f, 0.0f));
        }
        
        TEST_CASE("UserInterface::Result::LayoutResult defaults to Visible visibility", "[layout][result]")
        {
            LayoutResult r{};
            REQUIRE(r.Visibility.Value == Visibility::Visible);
        }
        
        TEST_CASE("UserInterface::Edges::Edges operator+ combines two edges element-wise", "[layout][edges]")
        {
            Edges a = Edges::Asymmetric(1.0f, 2.0f, 3.0f, 4.0f);
            Edges b = Edges::Asymmetric(10.0f, 20.0f, 30.0f, 40.0f);
            Edges result = a + b;
            REQUIRE(result.Top == Catch::Approx(11.0f));
            REQUIRE(result.Right == Catch::Approx(22.0f));
            REQUIRE(result.Bottom == Catch::Approx(33.0f));
            REQUIRE(result.Left == Catch::Approx(44.0f));
        }
        
        TEST_CASE("UserInterface::Edges::Edges operator* scales each edge by a scalar factor", "[layout][edges]")
        {
            Edges e = Edges::Asymmetric(2.0f, 4.0f, 6.0f, 8.0f);
            Edges result = e * 2.5f;
            REQUIRE(result.Top == Catch::Approx(5.0f));
            REQUIRE(result.Right == Catch::Approx(10.0f));
            REQUIRE(result.Bottom == Catch::Approx(15.0f));
            REQUIRE(result.Left == Catch::Approx(20.0f));
        }
        
        TEST_CASE("UserInterface::Edges::Edges operator/ divides each edge by a scalar factor", "[layout][edges]")
        {
            Edges e = Edges::Uniform(10.0f);
            Edges result = e / 4.0f;
            REQUIRE(result.Top == Catch::Approx(2.5f));
            REQUIRE(result.Right == Catch::Approx(2.5f));
            REQUIRE(result.Bottom == Catch::Approx(2.5f));
            REQUIRE(result.Left == Catch::Approx(2.5f));
        }
	}
}