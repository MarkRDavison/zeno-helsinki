#include <catch2/catch_test_macros.hpp>
#include <fakeit.hpp>
#include <helsinki/UserInterface/LayoutEngine.hpp>
#include "../TestHelpers.hpp"

namespace hl
{
	namespace tests
	{

        static void DoLayout(LayoutNode& a_Root, Vec2f a_AvailSize)
        {
            MeasureLayoutNode(a_Root, a_AvailSize);
            ArrangeLayoutNode(a_Root, Rectf{ .Origin = { 0.0f, 0.0f }, .Size = a_AvailSize });
        }

        static void RequireRect(const Rectf& a_Rect, Vec2f a_Origin, Vec2f a_Size)
        {
            REQUIRE(a_Rect.Origin[0] == Catch::Approx(a_Origin[0]).epsilon(1e-5f));
            REQUIRE(a_Rect.Origin[1] == Catch::Approx(a_Origin[1]).epsilon(1e-5f));
            REQUIRE(a_Rect.Size[0] == Catch::Approx(a_Size[0]).epsilon(1e-5f));
            REQUIRE(a_Rect.Size[1] == Catch::Approx(a_Size[1]).epsilon(1e-5f));
        }

        TEST_CASE("UserInterface::AlignRect::AlignRect TopLeft places content at the container origin", "[arrange][alignrect]")
        {
            Rectf container{ .Origin = { 100.0f, 50.0f }, .Size = { 200.0f, 100.0f } };
            Rectf result = AlignRect({ 40.0f, 20.0f }, container, EAlignment::TopLeft);
            RequireRect(result, { 100.0f, 50.0f }, { 40.0f, 20.0f });
        }

        TEST_CASE("UserInterface::AlignRect::AlignRect Center places content at the center of the container", "[arrange][alignrect]")
        {
            Rectf container{ .Origin = { 0.0f, 0.0f }, .Size = { 200.0f, 100.0f } };
            // content 40x20 → offset (80, 40)
            Rectf result = AlignRect({ 40.0f, 20.0f }, container, EAlignment::Center);
            RequireRect(result, { 80.0f, 40.0f }, { 40.0f, 20.0f });
        }

        TEST_CASE("UserInterface::AlignRect::AlignRect BottomRight places content at the bottom-right of the container", "[arrange][alignrect]")
        {
            Rectf container{ .Origin = { 0.0f, 0.0f }, .Size = { 200.0f, 100.0f } };
            // content 40x20 → offset (160, 80)
            Rectf result = AlignRect({ 40.0f, 20.0f }, container, EAlignment::BottomRight);
            RequireRect(result, { 160.0f, 80.0f }, { 40.0f, 20.0f });
        }

        TEST_CASE("UserInterface::AlignRect::AlignRect TopCenter places content horizontally centered at the top", "[arrange][alignrect]")
        {
            Rectf container{ .Origin = { 0.0f, 0.0f }, .Size = { 200.0f, 100.0f } };
            // content 40x20 → offset x=(200-40)/2=80, y=0
            Rectf result = AlignRect({ 40.0f, 20.0f }, container, EAlignment::TopCenter);
            RequireRect(result, { 80.0f, 0.0f }, { 40.0f, 20.0f });
        }

        TEST_CASE("UserInterface::AlignRect::AlignRect CenterLeft places content vertically centered on the left", "[arrange][alignrect]")
        {
            Rectf container{ .Origin = { 0.0f, 0.0f }, .Size = { 200.0f, 100.0f } };
            // content 40x20 → offset x=0, y=(100-20)/2=40
            Rectf result = AlignRect({ 40.0f, 20.0f }, container, EAlignment::CenterLeft);
            RequireRect(result, { 0.0f, 40.0f }, { 40.0f, 20.0f });
        }

        TEST_CASE("UserInterface::AlignRect::AlignRect BottomCenter places content horizontally centered at the bottom", "[arrange][alignrect]")
        {
            Rectf container{ .Origin = { 0.0f, 0.0f }, .Size = { 200.0f, 100.0f } };
            Rectf result = AlignRect({ 40.0f, 20.0f }, container, EAlignment::BottomCenter);
            RequireRect(result, { 80.0f, 80.0f }, { 40.0f, 20.0f });
        }

        TEST_CASE("UserInterface::AlignRect::AlignRect TopRight places content at the top-right corner", "[arrange][alignrect]")
        {
            Rectf container{ .Origin = { 10.0f, 10.0f }, .Size = { 100.0f, 80.0f } };
            // content 20x15 → offset x = 10 + (100-20) = 90, y = 10
            Rectf result = AlignRect({ 20.0f, 15.0f }, container, EAlignment::TopRight);
            RequireRect(result, { 90.0f, 10.0f }, { 20.0f, 15.0f });
        }

        TEST_CASE("UserInterface::AlignRect::AlignRect with non-zero container origin offsets result correctly", "[arrange][alignrect]")
        {
            Rectf container{ .Origin = { 50.0f, 30.0f }, .Size = { 100.0f, 60.0f } };
            Rectf result = AlignRect({ 100.0f, 60.0f }, container, EAlignment::Center);
            // Content exactly fills container — origin should equal container origin
            RequireRect(result, { 50.0f, 30.0f }, { 100.0f, 60.0f });
        }

        TEST_CASE("UserInterface::AlignCrossAxis::AlignCrossAxis TopLeft on horizontal layout places child at top", "[arrange][crossaxis]")
        {
            // Horizontal layout → cross axis is vertical → checking Top flag
            f32 pos = AlignCrossAxis(20.0f, 0.0f, 100.0f, EAlignment::TopLeft, /*isHz=*/true);
            REQUIRE(pos == Catch::Approx(0.0f));
        }

        TEST_CASE("UserInterface::AlignCrossAxis::AlignCrossAxis VCenter on horizontal layout centers child vertically", "[arrange][crossaxis]")
        {
            f32 pos = AlignCrossAxis(20.0f, 0.0f, 100.0f, EAlignment::Center, /*isHz=*/true);
            REQUIRE(pos == Catch::Approx(40.0f)); // (100-20)/2
        }

        TEST_CASE("UserInterface::AlignCrossAxis::AlignCrossAxis Bottom on horizontal layout places child at bottom", "[arrange][crossaxis]")
        {
            f32 pos = AlignCrossAxis(20.0f, 0.0f, 100.0f, EAlignment::BottomLeft, /*isHz=*/true);
            REQUIRE(pos == Catch::Approx(80.0f)); // 100-20
        }

        TEST_CASE("UserInterface::AlignCrossAxis::AlignCrossAxis Left on vertical layout places child at left edge", "[arrange][crossaxis]")
        {
            // Vertical layout → cross axis is horizontal → checking Left flag
            f32 pos = AlignCrossAxis(30.0f, 0.0f, 200.0f, EAlignment::TopLeft, /*isHz=*/false);
            REQUIRE(pos == Catch::Approx(0.0f));
        }

        TEST_CASE("UserInterface::AlignCrossAxis::AlignCrossAxis HCenter on vertical layout centers child horizontally", "[arrange][crossaxis]")
        {
            f32 pos = AlignCrossAxis(30.0f, 0.0f, 200.0f, EAlignment::Center, /*isHz=*/false);
            REQUIRE(pos == Catch::Approx(85.0f)); // (200-30)/2
        }

        TEST_CASE("UserInterface::AlignCrossAxis::AlignCrossAxis Right on vertical layout places child at right edge", "[arrange][crossaxis]")
        {
            f32 pos = AlignCrossAxis(30.0f, 0.0f, 200.0f, EAlignment::CenterRight, /*isHz=*/false);
            REQUIRE(pos == Catch::Approx(170.0f)); // 200-30
        }

        TEST_CASE("UserInterface::AlignCrossAxis::AlignCrossAxis with non-zero parent position offsets result", "[arrange][crossaxis]")
        {
            f32 pos = AlignCrossAxis(20.0f, 50.0f, 100.0f, EAlignment::Center, /*isHz=*/true);
            REQUIRE(pos == Catch::Approx(90.0f)); // 50 + (100-20)/2 = 50+40 = 90
        }

        TEST_CASE("UserInterface::ResolveAlign::ResolveAlign returns parent ChildAlign when child SelfAlign is Inherit", "[arrange][resolve]")
        {
            LayoutNode parent{};
            parent.Style.ChildAlign = EAlignment::Center;

            LayoutNode child{};
            child.Style.SelfAlign = EAlignment::Inherit;

            EAlignment result = ResolveAlign(child, parent);
            REQUIRE(result == EAlignment::Center);
        }

        TEST_CASE("UserInterface::ResolveAlign::ResolveAlign returns child SelfAlign when it is not Inherit", "[arrange][resolve]")
        {
            LayoutNode parent{};
            parent.Style.ChildAlign = EAlignment::Center;

            LayoutNode child{};
            child.Style.SelfAlign = EAlignment::BottomRight;

            EAlignment result = ResolveAlign(child, parent);
            REQUIRE(result == EAlignment::BottomRight);
        }

        TEST_CASE("UserInterface::ArrangeLayoutNode::ArrangeLayoutNode stores the allocated rect as FinalRect", "[arrange][LayoutNode]")
        {
            LayoutNode w{};
            Rectf rect{ .Origin = { 10.0f, 20.0f }, .Size = { 150.0f, 80.0f } };
            ArrangeLayoutNode(w, rect);
            RequireRect(w.Layout.FinalRect, { 10.0f, 20.0f }, { 150.0f, 80.0f });
        }

        TEST_CASE("UserInterface::ArrangeOverlay::ArrangeOverlay TopLeft child is placed at container origin", "[arrange][overlay]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.ChildAlign = EAlignment::TopLeft;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 50.0f;
            child.Style.FixedHeight = 30.0f;

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            RequireRect(child.Layout.FinalRect, { 0.0f, 0.0f }, { 50.0f, 30.0f });
        }

        TEST_CASE("UserInterface::ArrangeOverlay::ArrangeOverlay Center child is placed at the container center", "[arrange][overlay]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.ChildAlign = EAlignment::Center;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 40.0f;
            child.Style.FixedHeight = 20.0f;

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            // offset x = (200-40)/2 = 80, y = (100-20)/2 = 40
            RequireRect(child.Layout.FinalRect, { 80.0f, 40.0f }, { 40.0f, 20.0f });
        }

        TEST_CASE("UserInterface::ArrangeOverlay::ArrangeOverlay BottomRight child is placed at container bottom-right", "[arrange][overlay]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.ChildAlign = EAlignment::BottomRight;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 30.0f;
            child.Style.FixedHeight = 20.0f;

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            RequireRect(child.Layout.FinalRect, { 170.0f, 80.0f }, { 30.0f, 20.0f });
        }

        TEST_CASE("UserInterface::ArrangeOverlay::ArrangeOverlay child SelfAlign overrides parent ChildAlign", "[arrange][overlay]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.ChildAlign = EAlignment::TopLeft;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 40.0f;
            child.Style.FixedHeight = 20.0f;
            child.Style.SelfAlign = EAlignment::Center;

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            RequireRect(child.Layout.FinalRect, { 80.0f, 40.0f }, { 40.0f, 20.0f });
        }

        TEST_CASE("UserInterface::ArrangeOverlay::ArrangeOverlay multiple children each aligned independently", "[arrange][overlay]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 200.0f;
            parent.Style.FixedHeight = 100.0f;

            LayoutNode c1{};
            c1.Style.WidthMode = ESizingMode::Fixed; c1.Style.HeightMode = ESizingMode::Fixed;
            c1.Style.FixedWidth = 20.0f;             c1.Style.FixedHeight = 10.0f;
            c1.Style.SelfAlign = EAlignment::TopLeft;

            LayoutNode c2{};
            c2.Style.WidthMode = ESizingMode::Fixed; c2.Style.HeightMode = ESizingMode::Fixed;
            c2.Style.FixedWidth = 20.0f;             c2.Style.FixedHeight = 10.0f;
            c2.Style.SelfAlign = EAlignment::BottomRight;

            parent.PushBackChild(c1);
            parent.PushBackChild(c2);

            DoLayout(parent, { 200.0f, 100.0f });

            RequireRect(c1.Layout.FinalRect, { 0.0f,  0.0f }, { 20.0f, 10.0f });
            RequireRect(c2.Layout.FinalRect, { 180.0f, 90.0f }, { 20.0f, 10.0f });
        }

        TEST_CASE("UserInterface::ArrangeOverlay::ArrangeOverlay with padding reduces inner area for children", "[arrange][overlay]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.ChildAlign = EAlignment::TopLeft;
            parent.Style.Padding = Edges::Uniform(10.0f);

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 30.0f;
            child.Style.FixedHeight = 20.0f;

            parent.PushBackChild(child);

            DoLayout(parent, { 100.0f, 80.0f });

            // inner rect = 80x60 starting at (10,10)
            RequireRect(child.Layout.FinalRect, { 10.0f, 10.0f }, { 30.0f, 20.0f });
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Horizontal places children side by side from left", "[arrange][linear][horizontal]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.ChildAlign = EAlignment::TopLeft;

            LayoutNode c1{}; c1.Style.WidthMode = ESizingMode::Fixed; c1.Style.HeightMode = ESizingMode::Fixed;
            c1.Style.FixedWidth = 50.0f; c1.Style.FixedHeight = 30.0f;

            LayoutNode c2{}; c2.Style.WidthMode = ESizingMode::Fixed; c2.Style.HeightMode = ESizingMode::Fixed;
            c2.Style.FixedWidth = 80.0f; c2.Style.FixedHeight = 30.0f;

            parent.PushBackChild(c1);
            parent.PushBackChild(c2);

            DoLayout(parent, { 400.0f, 100.0f });

            // c1 at (0,0), c2 at (50,0)
            RequireRect(c1.Layout.FinalRect, { 0.0f,  0.0f }, { 50.0f, 30.0f });
            RequireRect(c2.Layout.FinalRect, { 50.0f, 0.0f }, { 80.0f, 30.0f });
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Horizontal with spacing adds gap between children", "[arrange][linear][horizontal]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.Spacing = 15.0f;
            parent.Style.ChildAlign = EAlignment::TopLeft;

            LayoutNode c1{}; c1.Style.WidthMode = ESizingMode::Fixed; c1.Style.HeightMode = ESizingMode::Fixed;
            c1.Style.FixedWidth = 40.0f; c1.Style.FixedHeight = 20.0f;

            LayoutNode c2{}; c2.Style.WidthMode = ESizingMode::Fixed; c2.Style.HeightMode = ESizingMode::Fixed;
            c2.Style.FixedWidth = 60.0f; c2.Style.FixedHeight = 20.0f;

            parent.PushBackChild(c1);
            parent.PushBackChild(c2);

            DoLayout(parent, { 400.0f, 100.0f });

            // c1 at 0, c2 at 0 + 15(spacing) + 40 = 55
            RequireRect(c1.Layout.FinalRect, { 0.0f,  0.0f }, { 40.0f, 20.0f });
            RequireRect(c2.Layout.FinalRect, { 55.0f, 0.0f }, { 60.0f, 20.0f });
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Horizontal cross-axis VCenter aligns child vertically", "[arrange][linear][horizontal]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.ChildAlign = EAlignment::Center;   // VCenter + HCenter

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 50.0f;
            child.Style.FixedHeight = 20.0f;

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            // Cross-axis (vertical): (100 - 20)/2 = 40
            REQUIRE(child.Layout.FinalRect.Origin[1] == Catch::Approx(40.0f));
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Horizontal child with VStretch fills cross axis", "[arrange][linear][horizontal]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 50.0f;
            child.Style.FixedHeight = 20.0f;
            child.Style.SelfAlign = EAlignment::VStretch;

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            REQUIRE(child.Layout.FinalRect.Size[1] == Catch::Approx(100.0f));
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Horizontal with padding offsets children inside inner rect", "[arrange][linear][horizontal]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.ChildAlign = EAlignment::TopLeft;
            parent.Style.Padding = Edges::Uniform(10.0f);

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 50.0f;
            child.Style.FixedHeight = 30.0f;

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            // inner rect starts at (10,10)
            RequireRect(child.Layout.FinalRect, { 10.0f, 10.0f }, { 50.0f, 30.0f });
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Horizontal single-child margin shifts position and shrinks size", "[arrange][linear][horizontal]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.ChildAlign = EAlignment::TopLeft;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 60.0f;
            child.Style.FixedHeight = 40.0f;
            child.Style.Margin = Edges::Asymmetric(5.0f, 10.0f, 5.0f, 8.0f); // top=5, right=10, bottom=5, left=8

            parent.PushBackChild(child);

            DoLayout(parent, { 300.0f, 100.0f });

            // origin shifted by margin: x += left=8, y += top=5; size shrunk by margin: w -= 8+10=18, h -= 5+5=10
            RequireRect(child.Layout.FinalRect, { 8.0f, 5.0f }, { 42.0f, 30.0f });
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Horizontal multiple children cursor advances correctly", "[arrange][linear][horizontal]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.ChildAlign = EAlignment::TopLeft;
            parent.Style.Spacing = 5.0f;

            LayoutNode c1{}; c1.Style.WidthMode = ESizingMode::Fixed; c1.Style.HeightMode = ESizingMode::Fixed;
            c1.Style.FixedWidth = 20.0f; c1.Style.FixedHeight = 10.0f;

            LayoutNode c2{}; c2.Style.WidthMode = ESizingMode::Fixed; c2.Style.HeightMode = ESizingMode::Fixed;
            c2.Style.FixedWidth = 30.0f; c2.Style.FixedHeight = 10.0f;

            LayoutNode c3{}; c3.Style.WidthMode = ESizingMode::Fixed; c3.Style.HeightMode = ESizingMode::Fixed;
            c3.Style.FixedWidth = 40.0f; c3.Style.FixedHeight = 10.0f;

            parent.PushBackChild(c1);
            parent.PushBackChild(c2);
            parent.PushBackChild(c3);

            DoLayout(parent, { 400.0f, 100.0f });

            // c1 at 0, c2 at 0+5+20=25, c3 at 25+5+30=60
            REQUIRE(c1.Layout.FinalRect.Origin[0] == Catch::Approx(0.0f));
            REQUIRE(c2.Layout.FinalRect.Origin[0] == Catch::Approx(25.0f));
            REQUIRE(c3.Layout.FinalRect.Origin[0] == Catch::Approx(60.0f));
        }
        
        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Vertical places children top to bottom", "[arrange][linear][vertical]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Vertical;
            parent.Style.ChildAlign = EAlignment::TopLeft;

            LayoutNode c1{}; c1.Style.WidthMode = ESizingMode::Fixed; c1.Style.HeightMode = ESizingMode::Fixed;
            c1.Style.FixedWidth = 50.0f; c1.Style.FixedHeight = 30.0f;

            LayoutNode c2{}; c2.Style.WidthMode = ESizingMode::Fixed; c2.Style.HeightMode = ESizingMode::Fixed;
            c2.Style.FixedWidth = 50.0f; c2.Style.FixedHeight = 40.0f;

            parent.PushBackChild(c1);
            parent.PushBackChild(c2);

            DoLayout(parent, { 200.0f, 400.0f });

            RequireRect(c1.Layout.FinalRect, { 0.0f, 0.0f }, { 50.0f, 30.0f });
            RequireRect(c2.Layout.FinalRect, { 0.0f, 30.0f }, { 50.0f, 40.0f });
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Vertical with spacing adds gap between children", "[arrange][linear][vertical]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Vertical;
            parent.Style.Spacing = 10.0f;
            parent.Style.ChildAlign = EAlignment::TopLeft;

            LayoutNode c1{}; c1.Style.WidthMode = ESizingMode::Fixed; c1.Style.HeightMode = ESizingMode::Fixed;
            c1.Style.FixedWidth = 50.0f; c1.Style.FixedHeight = 20.0f;

            LayoutNode c2{}; c2.Style.WidthMode = ESizingMode::Fixed; c2.Style.HeightMode = ESizingMode::Fixed;
            c2.Style.FixedWidth = 50.0f; c2.Style.FixedHeight = 30.0f;

            parent.PushBackChild(c1);
            parent.PushBackChild(c2);

            DoLayout(parent, { 200.0f, 400.0f });

            // c1 at y=0, c2 at y = 0+10(spacing)+20 = 30
            RequireRect(c1.Layout.FinalRect, { 0.0f, 0.0f }, { 50.0f, 20.0f });
            RequireRect(c2.Layout.FinalRect, { 0.0f, 30.0f }, { 50.0f, 30.0f });
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Vertical cross-axis HCenter centers child horizontally", "[arrange][linear][vertical]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Vertical;
            parent.Style.ChildAlign = EAlignment::Center;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 40.0f;
            child.Style.FixedHeight = 20.0f;

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            // cross axis is horizontal: (200 - 40)/2 = 80
            REQUIRE(child.Layout.FinalRect.Origin[0] == Catch::Approx(80.0f));
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Vertical child with HStretch fills cross axis", "[arrange][linear][vertical]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Vertical;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 40.0f;
            child.Style.FixedHeight = 20.0f;
            child.Style.SelfAlign = EAlignment::HStretch;

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            REQUIRE(child.Layout.FinalRect.Size[0] == Catch::Approx(200.0f));
        }

        TEST_CASE("UserInterface::FlexGrow::FlexGrow single child takes all leftover space", "[arrange][flex]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 300.0f;
            parent.Style.FixedHeight = 50.0f;

            LayoutNode c1{}; c1.Style.WidthMode = ESizingMode::Fixed; c1.Style.HeightMode = ESizingMode::Fixed;
            c1.Style.FixedWidth = 100.0f; c1.Style.FixedHeight = 50.0f;

            LayoutNode c2{}; c2.Style.WidthMode = ESizingMode::Fixed; c2.Style.HeightMode = ESizingMode::Fixed;
            c2.Style.FixedWidth = 50.0f; c2.Style.FixedHeight = 50.0f;
            c2.Style.FlexGrow = 1.0f; // takes leftover

            parent.PushBackChild(c1);
            parent.PushBackChild(c2);

            DoLayout(parent, { 300.0f, 50.0f });

            // available = 300, fixed = 100+50 = 150, leftover = 150 → c2 gets 50+150 = 200
            REQUIRE(c2.Layout.FinalRect.Size[0] == Catch::Approx(200.0f));
        }

        TEST_CASE("UserInterface::FlexGrow::FlexGrow two equal-weight children split leftover equally", "[arrange][flex]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 300.0f;
            parent.Style.FixedHeight = 50.0f;

            LayoutNode c1{}; c1.Style.WidthMode = ESizingMode::Fixed; c1.Style.HeightMode = ESizingMode::Fixed;
            c1.Style.FixedWidth = 0.0f; c1.Style.FixedHeight = 50.0f;
            c1.Style.FlexGrow = 1.0f;

            LayoutNode c2{}; c2.Style.WidthMode = ESizingMode::Fixed; c2.Style.HeightMode = ESizingMode::Fixed;
            c2.Style.FixedWidth = 0.0f; c2.Style.FixedHeight = 50.0f;
            c2.Style.FlexGrow = 1.0f;

            parent.PushBackChild(c1);
            parent.PushBackChild(c2);

            DoLayout(parent, { 300.0f, 50.0f });

            // both start at 0, leftover = 300, each gets 150
            REQUIRE(c1.Layout.FinalRect.Size[0] == Catch::Approx(150.0f));
            REQUIRE(c2.Layout.FinalRect.Size[0] == Catch::Approx(150.0f));
        }

        TEST_CASE("UserInterface::FlexGrow::FlexGrow 2:1 ratio distributes leftover proportionally", "[arrange][flex]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 300.0f;
            parent.Style.FixedHeight = 50.0f;

            LayoutNode c1{}; c1.Style.WidthMode = ESizingMode::Fixed; c1.Style.HeightMode = ESizingMode::Fixed;
            c1.Style.FixedWidth = 0.0f; c1.Style.FixedHeight = 50.0f;
            c1.Style.FlexGrow = 2.0f;

            LayoutNode c2{}; c2.Style.WidthMode = ESizingMode::Fixed; c2.Style.HeightMode = ESizingMode::Fixed;
            c2.Style.FixedWidth = 0.0f; c2.Style.FixedHeight = 50.0f;
            c2.Style.FlexGrow = 1.0f;

            parent.PushBackChild(c1);
            parent.PushBackChild(c2);

            DoLayout(parent, { 300.0f, 50.0f });

            // leftover = 300, c1 gets 2/3 * 300 = 200, c2 gets 1/3 * 300 = 100
            REQUIRE(c1.Layout.FinalRect.Size[0] == Catch::Approx(200.0f));
            REQUIRE(c2.Layout.FinalRect.Size[0] == Catch::Approx(100.0f));
        }

        TEST_CASE("UserInterface::FlexGrow::FlexGrow is clamped by SizeConstraints max", "[arrange][flex]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 300.0f;
            parent.Style.FixedHeight = 50.0f;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 0.0f;
            child.Style.FixedHeight = 50.0f;
            child.Style.FlexGrow = 1.0f;
            child.Style.SizeConstraints = Constraints::AtMost({ 100.0f, 100.0f });

            parent.PushBackChild(child);

            DoLayout(parent, { 300.0f, 50.0f });

            // Would get 300, but clamped to max 100
            REQUIRE(child.Layout.FinalRect.Size[0] == Catch::Approx(100.0f));
        }

        TEST_CASE("UserInterface::FlexGrow::FlexGrow Vertical: child grows along vertical axis", "[arrange][flex][vertical]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Vertical;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 100.0f;
            parent.Style.FixedHeight = 200.0f;

            LayoutNode c1{}; c1.Style.WidthMode = ESizingMode::Fixed; c1.Style.HeightMode = ESizingMode::Fixed;
            c1.Style.FixedWidth = 100.0f; c1.Style.FixedHeight = 50.0f;

            LayoutNode c2{}; c2.Style.WidthMode = ESizingMode::Fixed; c2.Style.HeightMode = ESizingMode::Fixed;
            c2.Style.FixedWidth = 100.0f; c2.Style.FixedHeight = 0.0f;
            c2.Style.FlexGrow = 1.0f;

            parent.PushBackChild(c1);
            parent.PushBackChild(c2);

            DoLayout(parent, { 100.0f, 200.0f });

            // available = 200, fixed = 50, leftover = 150 → c2 gets 150
            REQUIRE(c2.Layout.FinalRect.Size[1] == Catch::Approx(150.0f));
        }

        TEST_CASE("UserInterface::ArrangeAnchored::ArrangeAnchored TopLeft point anchor places child at top-left of parent", "[arrange][anchored]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 200.0f;
            parent.Style.FixedHeight = 100.0f;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 40.0f;
            child.Style.FixedHeight = 20.0f;
            child.Style.PositionMode = EPositioningMode::Anchored;
            child.Style.Anchor = Anchor::TopLeft();  // Min=(0,0), Pivot=(0,0)

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            RequireRect(child.Layout.FinalRect, { 0.0f, 0.0f }, { 40.0f, 20.0f });
        }

        TEST_CASE("UserInterface::ArrangeAnchored::ArrangeAnchored Center point anchor places child at parent center", "[arrange][anchored]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 200.0f;
            parent.Style.FixedHeight = 100.0f;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 40.0f;
            child.Style.FixedHeight = 20.0f;
            child.Style.PositionMode = EPositioningMode::Anchored;
            child.Style.Anchor = Anchor::Center(); // Min=(0.5,0.5), Pivot=(0.5,0.5)

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            // anchorPoint = (100, 50); origin = (100,50) - (40*0.5, 20*0.5) = (80, 40)
            RequireRect(child.Layout.FinalRect, { 80.0f, 40.0f }, { 40.0f, 20.0f });
        }

        TEST_CASE("UserInterface::ArrangeAnchored::ArrangeAnchored BottomRight point anchor places child at bottom-right", "[arrange][anchored]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 200.0f;
            parent.Style.FixedHeight = 100.0f;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 40.0f;
            child.Style.FixedHeight = 20.0f;
            child.Style.PositionMode = EPositioningMode::Anchored;
            child.Style.Anchor = Anchor::BottomRight(); // Min=(1,1), Pivot=(1,1)

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            // anchorPoint = (200,100); origin = (200,100) - (40*1, 20*1) = (160, 80)
            RequireRect(child.Layout.FinalRect, { 160.0f, 80.0f }, { 40.0f, 20.0f });
        }

        TEST_CASE("UserInterface::ArrangeAnchored::ArrangeAnchored StretchAll fills the entire parent", "[arrange][anchored]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 200.0f;
            parent.Style.FixedHeight = 100.0f;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 0.0f;
            child.Style.FixedHeight = 0.0f;
            child.Style.PositionMode = EPositioningMode::Anchored;
            child.Style.Anchor = Anchor::StretchAll(); // Min=(0,0), Max=(1,1), offset=(0,0)

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            // finalMin = (0,0) + (200,100)*(0,0) + (0,0) = (0,0)
            // finalMax = (0,0) + (200,100)*(1,1) - (0,0) = (200,100)
            RequireRect(child.Layout.FinalRect, { 0.0f, 0.0f }, { 200.0f, 100.0f });
        }

        TEST_CASE("UserInterface::ArrangeAnchored::ArrangeAnchored with pixel offset shifts position", "[arrange][anchored]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 200.0f;
            parent.Style.FixedHeight = 100.0f;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 40.0f;
            child.Style.FixedHeight = 20.0f;
            child.Style.PositionMode = EPositioningMode::Anchored;
            // Point anchor at top-left with a pixel offset of (10, 5)
            child.Style.Anchor = { .Min = { 0.0f, 0.0f }, .Max = { 0.0f, 0.0f }, .Pivot = { 0.0f, 0.0f }, .Offset = { 10.0f, 5.0f } };

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            // anchorPoint = (0,0) + offset(10,5) = (10,5); origin = (10,5) - (0,0) = (10,5)
            RequireRect(child.Layout.FinalRect, { 10.0f, 5.0f }, { 40.0f, 20.0f });
        }

        TEST_CASE("UserInterface::ArrangeAnchored::ArrangeAnchored StretchTop stretches across top edge with zero height", "[arrange][anchored]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 200.0f;
            parent.Style.FixedHeight = 100.0f;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 0.0f;
            child.Style.FixedHeight = 0.0f;
            child.Style.PositionMode = EPositioningMode::Anchored;
            child.Style.Anchor = Anchor::StretchTop(); // Min=(0,0), Max=(1,0)

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            // stretch: finalMin=(0,0), finalMax=(200,0) → size=(200,0)
            REQUIRE(child.Layout.FinalRect.Size[0] == Catch::Approx(200.0f));
            REQUIRE(child.Layout.FinalRect.Size[1] == Catch::Approx(0.0f));
            REQUIRE(child.Layout.FinalRect.Origin[1] == Catch::Approx(0.0f));
        }

        TEST_CASE("UserInterface::ArrangeAnchored::ArrangeAnchored anchored child does not affect parent content size", "[arrange][anchored]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.WidthMode = ESizingMode::Content;
            parent.Style.HeightMode = ESizingMode::Content;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 300.0f;
            child.Style.FixedHeight = 200.0f;
            child.Style.PositionMode = EPositioningMode::Anchored;
            child.Style.Anchor = Anchor::TopLeft();

            parent.PushBackChild(child);

            Vec2f desired = MeasureLayoutNode(parent, { 400.0f, 300.0f });
            // Anchored child is excluded from content measurement → parent stays 0x0
            RequireApproxEqual(desired, Vec2f(0.0f, 0.0f));
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Collapsed child is skipped and does not advance cursor", "[arrange][visibility]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.ChildAlign = EAlignment::TopLeft;

            LayoutNode c1{}; c1.Style.WidthMode = ESizingMode::Fixed; c1.Style.HeightMode = ESizingMode::Fixed;
            c1.Style.FixedWidth = 50.0f; c1.Style.FixedHeight = 30.0f;

            LayoutNode c2{}; c2.Style.WidthMode = ESizingMode::Fixed; c2.Style.HeightMode = ESizingMode::Fixed;
            c2.Style.FixedWidth = 50.0f; c2.Style.FixedHeight = 30.0f;
            c2.Layout.Visibility = { Visibility::Collapsed };

            LayoutNode c3{}; c3.Style.WidthMode = ESizingMode::Fixed; c3.Style.HeightMode = ESizingMode::Fixed;
            c3.Style.FixedWidth = 70.0f; c3.Style.FixedHeight = 30.0f;

            parent.PushBackChild(c1);
            parent.PushBackChild(c2);
            parent.PushBackChild(c3);

            // Measure first so DesiredSizes are set
            MeasureLayoutNode(parent, { 400.0f, 100.0f });
            ArrangeLayoutNode(parent, { .Origin = { 0.0f, 0.0f }, .Size = { 400.0f, 100.0f } });

            // c2 is collapsed, so c3 should start right after c1 (no gap for c2)
            REQUIRE(c1.Layout.FinalRect.Origin[0] == Catch::Approx(0.0f));
            REQUIRE(c3.Layout.FinalRect.Origin[0] == Catch::Approx(50.0f));
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Hidden child still occupies space in layout", "[arrange][visibility]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.ChildAlign = EAlignment::TopLeft;

            LayoutNode c1{}; c1.Style.WidthMode = ESizingMode::Fixed; c1.Style.HeightMode = ESizingMode::Fixed;
            c1.Style.FixedWidth = 50.0f; c1.Style.FixedHeight = 30.0f;

            LayoutNode c2{}; c2.Style.WidthMode = ESizingMode::Fixed; c2.Style.HeightMode = ESizingMode::Fixed;
            c2.Style.FixedWidth = 60.0f; c2.Style.FixedHeight = 30.0f;
            c2.Layout.Visibility = { Visibility::Hidden }; // affects layout, not rendered

            LayoutNode c3{}; c3.Style.WidthMode = ESizingMode::Fixed; c3.Style.HeightMode = ESizingMode::Fixed;
            c3.Style.FixedWidth = 40.0f; c3.Style.FixedHeight = 30.0f;

            parent.PushBackChild(c1);
            parent.PushBackChild(c2);
            parent.PushBackChild(c3);

            MeasureLayoutNode(parent, { 400.0f, 100.0f });
            ArrangeLayoutNode(parent, { .Origin = { 0.0f, 0.0f }, .Size = { 400.0f, 100.0f } });

            // c1@0, c2@50, c3@110
            REQUIRE(c1.Layout.FinalRect.Origin[0] == Catch::Approx(0.0f));
            REQUIRE(c2.Layout.FinalRect.Origin[0] == Catch::Approx(50.0f));
            REQUIRE(c3.Layout.FinalRect.Origin[0] == Catch::Approx(110.0f));
        }

        TEST_CASE("UserInterface::ArrangeLayoutNode::ArrangeLayoutNode deeply nested: 3 levels of Overlay LayoutNodes set FinalRect", "[arrange][nested]")
        {
            LayoutNode l1{};
            l1.Style.LayoutType = ELayoutType::Overlay;
            l1.Style.WidthMode = ESizingMode::Fixed; l1.Style.HeightMode = ESizingMode::Fixed;
            l1.Style.FixedWidth = 200.0f; l1.Style.FixedHeight = 200.0f;

            LayoutNode l2{};
            l2.Style.LayoutType = ELayoutType::Overlay;
            l2.Style.WidthMode = ESizingMode::Fixed; l2.Style.HeightMode = ESizingMode::Fixed;
            l2.Style.FixedWidth = 100.0f; l2.Style.FixedHeight = 100.0f;
            l2.Style.SelfAlign = EAlignment::Center;

            LayoutNode l3{};
            l3.Style.WidthMode = ESizingMode::Fixed; l3.Style.HeightMode = ESizingMode::Fixed;
            l3.Style.FixedWidth = 40.0f; l3.Style.FixedHeight = 40.0f;
            l3.Style.SelfAlign = EAlignment::Center;

            l2.PushBackChild(l3);
            l1.PushBackChild(l2);

            DoLayout(l1, { 200.0f, 200.0f });

            // l2 centered in l1 200x200 → origin (50, 50)
            RequireRect(l2.Layout.FinalRect, { 50.0f, 50.0f }, { 100.0f, 100.0f });

            // l3 centered in l2 100x100 at (50,50) → origin (50+(100-40)/2, 50+(100-40)/2) = (80, 80)
            RequireRect(l3.Layout.FinalRect, { 80.0f, 80.0f }, { 40.0f, 40.0f });
        }

        TEST_CASE("UserInterface::MeasureLayoutNode::MeasureLayoutNode Collapsed LayoutNode has zero desired size and empty FinalRect after full layout", "[arrange][visibility][roundtrip]")
        {
            LayoutNode w{};
            w.Style.WidthMode = ESizingMode::Fixed;
            w.Style.HeightMode = ESizingMode::Fixed;
            w.Style.FixedWidth = 100.0f;
            w.Style.FixedHeight = 50.0f;
            w.Layout.Visibility = { Visibility::Collapsed };

            Vec2f desired = MeasureLayoutNode(w, { 500.0f, 500.0f });
            RequireApproxEqual(desired, Vec2f(0.0f, 0.0f));
        }

        TEST_CASE("UserInterface::FlexGrow::FlexGrow respects padding: leftover excludes padding from available space", "[arrange][flex][padding]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 300.0f;
            parent.Style.FixedHeight = 50.0f;
            parent.Style.Padding = Edges::Symmetric(25.0f, 0.0f); // 25px each side = 50px total H padding

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 0.0f;
            child.Style.FixedHeight = 50.0f;
            child.Style.FlexGrow = 1.0f;

            parent.PushBackChild(child);

            DoLayout(parent, { 300.0f, 50.0f });

            // inner width = 300 - 50 = 250; child starts at (25, 0) and gets 250 width
            REQUIRE(child.Layout.FinalRect.Origin[0] == Catch::Approx(25.0f));
            REQUIRE(child.Layout.FinalRect.Size[0] == Catch::Approx(250.0f));
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Horizontal margin and spacing interact correctly", "[arrange][linear][margin][spacing]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.Spacing = 10.0f;
            parent.Style.ChildAlign = EAlignment::TopLeft;

            LayoutNode c1{}; c1.Style.WidthMode = ESizingMode::Fixed; c1.Style.HeightMode = ESizingMode::Fixed;
            c1.Style.FixedWidth = 50.0f; c1.Style.FixedHeight = 30.0f;
            c1.Style.Margin = Edges::Asymmetric(0.0f, 5.0f, 0.0f, 5.0f); // top=0, right=5, bottom=0, left=5

            LayoutNode c2{}; c2.Style.WidthMode = ESizingMode::Fixed; c2.Style.HeightMode = ESizingMode::Fixed;
            c2.Style.FixedWidth = 60.0f; c2.Style.FixedHeight = 30.0f;

            parent.PushBackChild(c1);
            parent.PushBackChild(c2);

            DoLayout(parent, { 400.0f, 100.0f });

            // c1: cursor starts at 0; origin x = 0 + margin.left(5) = 5; size.x = 50 - margin.H(10) = 40
            // After c1: cursor += spacing(10) + (c1.DesiredSize[0]=50 + c1.margin.H=10) = 0 + 10 + 60 = 70
            // c2: origin x = 70 + margin.left(0) = 70; size.x = 60
            REQUIRE(c1.Layout.FinalRect.Origin[0] == Catch::Approx(5.0f));
            REQUIRE(c1.Layout.FinalRect.Size[0] == Catch::Approx(40.0f));
            REQUIRE(c2.Layout.FinalRect.Origin[0] == Catch::Approx(70.0f));
            REQUIRE(c2.Layout.FinalRect.Size[0] == Catch::Approx(60.0f));
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Horizontal Content-sized parent resizes correctly after full Measure+Arrange round-trip", "[arrange][roundtrip]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.WidthMode = ESizingMode::Content;
            parent.Style.HeightMode = ESizingMode::Content;
            parent.Style.Spacing = 5.0f;

            LayoutNode c1{}; c1.Style.WidthMode = ESizingMode::Fixed; c1.Style.HeightMode = ESizingMode::Fixed;
            c1.Style.FixedWidth = 100.0f; c1.Style.FixedHeight = 40.0f;

            LayoutNode c2{}; c2.Style.WidthMode = ESizingMode::Fixed; c2.Style.HeightMode = ESizingMode::Fixed;
            c2.Style.FixedWidth = 80.0f; c2.Style.FixedHeight = 60.0f;

            parent.PushBackChild(c1);
            parent.PushBackChild(c2);

            DoLayout(parent, { 1000.0f, 1000.0f });

            // Parent desired: w=100+5+80=185, h=max(40,60)=60
            RequireApproxEqual(parent.Layout.DesiredSize, Vec2f(185.0f, 60.0f));

            // c1 at (0,0), c2 at (100+5, 0) = (105, 0) — but they are laid out inside a rect of 1000x1000
            REQUIRE(c1.Layout.FinalRect.Origin[0] == Catch::Approx(0.0f));
            REQUIRE(c2.Layout.FinalRect.Origin[0] == Catch::Approx(105.0f));
        }
        
        TEST_CASE("UserInterface::ArrangeAnchored::ArrangeAnchored StretchBottom stretches across bottom edge with zero height", "[arrange][anchored]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 200.0f;
            parent.Style.FixedHeight = 100.0f;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 0.0f;
            child.Style.FixedHeight = 0.0f;
            child.Style.PositionMode = EPositioningMode::Anchored;
            child.Style.Anchor = Anchor::StretchBottom(); // Min=(0,1), Max=(1,1)

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            // Stretch bottom edge: finalMin=(0,100), finalMax=(200,100) → size=(200,0)
            REQUIRE(child.Layout.FinalRect.Size[0] == Catch::Approx(200.0f));
            REQUIRE(child.Layout.FinalRect.Size[1] == Catch::Approx(0.0f));
            REQUIRE(child.Layout.FinalRect.Origin[1] == Catch::Approx(100.0f));
        }

        TEST_CASE("UserInterface::ArrangeAnchored::ArrangeAnchored StretchLeft stretches along left edge", "[arrange][anchored]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 200.0f;
            parent.Style.FixedHeight = 100.0f;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 0.0f;
            child.Style.FixedHeight = 0.0f;
            child.Style.PositionMode = EPositioningMode::Anchored;
            child.Style.Anchor = Anchor::StretchLeft(); // Min=(0,0), Max=(0,1)

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            // Stretch left edge: x is a point anchor (stretchX=false), y stretches full height
            REQUIRE(child.Layout.FinalRect.Size[1] == Catch::Approx(100.0f));
            REQUIRE(child.Layout.FinalRect.Size[0] == Catch::Approx(0.0f));
            REQUIRE(child.Layout.FinalRect.Origin[0] == Catch::Approx(0.0f));
        }

        TEST_CASE("UserInterface::ArrangeAnchored::ArrangeAnchored StretchRight stretches along right edge", "[arrange][anchored]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 200.0f;
            parent.Style.FixedHeight = 100.0f;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 0.0f;
            child.Style.FixedHeight = 0.0f;
            child.Style.PositionMode = EPositioningMode::Anchored;
            child.Style.Anchor = Anchor::StretchRight(); // Min=(1,0), Max=(1,1)

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            // Stretch right edge: x is a point anchor at 1.0, y stretches full height
            REQUIRE(child.Layout.FinalRect.Size[1] == Catch::Approx(100.0f));
            REQUIRE(child.Layout.FinalRect.Size[0] == Catch::Approx(0.0f));
            REQUIRE(child.Layout.FinalRect.Origin[0] == Catch::Approx(200.0f));
        }

        TEST_CASE("UserInterface::ArrangeAnchored::ArrangeAnchored StretchAll with inward offset shrinks the stretch rect symmetrically", "[arrange][anchored]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 200.0f;
            parent.Style.FixedHeight = 100.0f;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 0.0f;
            child.Style.FixedHeight = 0.0f;
            child.Style.PositionMode = EPositioningMode::Anchored;
            // StretchAll with offset=(10,5) pushes each edge inward by the offset value
            child.Style.Anchor = { .Min = { 0.0f, 0.0f }, .Max = { 1.0f, 1.0f }, .Pivot = { 0.5f, 0.5f }, .Offset = { 10.0f, 5.0f } };

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            // X: origin = 0 + 200*0 + 10 = 10; right = 0 + 200*1 - 10 = 190; w = 180
            // Y: origin = 0 + 100*0 +  5 =  5; bottom = 0 + 100*1 -  5 =  95; h =  90
            RequireRect(child.Layout.FinalRect, { 10.0f, 5.0f }, { 180.0f, 90.0f });
        }

        TEST_CASE("UserInterface::ArrangeAnchored::ArrangeAnchored with non-zero parent container origin positions child correctly", "[arrange][anchored]")
        {
            // Parent rect starts at (50, 30), not at (0,0)
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 200.0f;
            parent.Style.FixedHeight = 100.0f;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 40.0f;
            child.Style.FixedHeight = 20.0f;
            child.Style.PositionMode = EPositioningMode::Anchored;
            child.Style.Anchor = Anchor::Center(); // Min=Max=(0.5,0.5), Pivot=(0.5,0.5)

            parent.PushBackChild(child);

            // Arrange the parent rect starting at (50,30)
            MeasureLayoutNode(parent, { 200.0f, 100.0f });
            ArrangeLayoutNode(parent, { .Origin = { 50.0f, 30.0f }, .Size = { 200.0f, 100.0f } });

            // anchorX = 50 + 200*0.5 = 150; originX = 150 - 40*0.5 = 130
            // anchorY = 30 + 100*0.5 =  80; originY =  80 - 20*0.5 =  70
            RequireRect(child.Layout.FinalRect, { 130.0f, 70.0f }, { 40.0f, 20.0f });
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Horizontal anchored children do not advance the flow cursor", "[arrange][linear][anchored]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.ChildAlign = EAlignment::TopLeft;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 400.0f;
            parent.Style.FixedHeight = 100.0f;

            LayoutNode c1{}; c1.Style.WidthMode = ESizingMode::Fixed; c1.Style.HeightMode = ESizingMode::Fixed;
            c1.Style.FixedWidth = 50.0f; c1.Style.FixedHeight = 30.0f;

            // Anchored child – should NOT advance the horizontal cursor
            LayoutNode anchored{};
            anchored.Style.WidthMode = ESizingMode::Fixed;
            anchored.Style.HeightMode = ESizingMode::Fixed;
            anchored.Style.FixedWidth = 200.0f;
            anchored.Style.FixedHeight = 200.0f;
            anchored.Style.PositionMode = EPositioningMode::Anchored;
            anchored.Style.Anchor = Anchor::TopLeft();

            LayoutNode c3{}; c3.Style.WidthMode = ESizingMode::Fixed; c3.Style.HeightMode = ESizingMode::Fixed;
            c3.Style.FixedWidth = 40.0f; c3.Style.FixedHeight = 30.0f;

            parent.PushBackChild(c1);
            parent.PushBackChild(anchored);
            parent.PushBackChild(c3);

            DoLayout(parent, { 400.0f, 100.0f });

            // c1 at x=0, c3 immediately after c1 at x=50 (anchored child has no effect on flow)
            REQUIRE(c1.Layout.FinalRect.Origin[0] == Catch::Approx(0.0f));
            REQUIRE(c3.Layout.FinalRect.Origin[0] == Catch::Approx(50.0f));
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear FlexGrow with spacing: spacing reduces available flex space", "[arrange][flex][spacing]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 300.0f;
            parent.Style.FixedHeight = 50.0f;
            parent.Style.Spacing = 10.0f;

            LayoutNode c1{}; c1.Style.WidthMode = ESizingMode::Fixed; c1.Style.HeightMode = ESizingMode::Fixed;
            c1.Style.FixedWidth = 50.0f; c1.Style.FixedHeight = 50.0f;

            LayoutNode c2{}; c2.Style.WidthMode = ESizingMode::Fixed; c2.Style.HeightMode = ESizingMode::Fixed;
            c2.Style.FixedWidth = 0.0f; c2.Style.FixedHeight = 50.0f;
            c2.Style.FlexGrow = 1.0f;

            parent.PushBackChild(c1);
            parent.PushBackChild(c2);

            DoLayout(parent, { 300.0f, 50.0f });

            // available = 300 - spacing*(numFlow-1) = 300 - 10*1 = 290
            // totalFixed = 50 (c1)
            // leftover = 290 - 50 = 240 → c2 gets 240
            REQUIRE(c2.Layout.FinalRect.Size[0] == Catch::Approx(240.0f));
            REQUIRE(c2.Layout.FinalRect.Origin[0] == Catch::Approx(60.0f)); // 0 + spacing(10) + c1(50) = 60
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Vertical single-child margin shifts position and shrinks size", "[arrange][linear][vertical]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Vertical;
            parent.Style.ChildAlign = EAlignment::TopLeft;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Fixed;
            child.Style.FixedWidth = 80.0f;
            child.Style.FixedHeight = 60.0f;
            child.Style.Margin = Edges::Asymmetric(8.0f, 0.0f, 12.0f, 0.0f); // top=8, bottom=12

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 300.0f });

            // origin y += margin.top=8; size.h -= margin.vertical=20 → h=40
            RequireRect(child.Layout.FinalRect, { 0.0f, 8.0f }, { 80.0f, 40.0f });
        }

        TEST_CASE("UserInterface::ArrangeOverlay::ArrangeOverlay Collapsed child gets zero-size FinalRect and non-collapsed sibling is placed correctly", "[arrange][overlay][visibility]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 200.0f;
            parent.Style.FixedHeight = 100.0f;

            LayoutNode collapsed{};
            collapsed.Style.WidthMode = ESizingMode::Fixed;
            collapsed.Style.HeightMode = ESizingMode::Fixed;
            collapsed.Style.FixedWidth = 100.0f;
            collapsed.Style.FixedHeight = 50.0f;
            collapsed.Layout.Visibility = { Visibility::Collapsed };

            LayoutNode visible{};
            visible.Style.WidthMode = ESizingMode::Fixed;
            visible.Style.HeightMode = ESizingMode::Fixed;
            visible.Style.FixedWidth = 40.0f;
            visible.Style.FixedHeight = 20.0f;
            visible.Style.SelfAlign = EAlignment::BottomRight;

            parent.PushBackChild(collapsed);
            parent.PushBackChild(visible);

            DoLayout(parent, { 200.0f, 100.0f });

            // Collapsed child has DesiredSize=0×0 and gets a 0×0 FinalRect
            REQUIRE(collapsed.Layout.DesiredSize[0] == Catch::Approx(0.0f));
            REQUIRE(collapsed.Layout.DesiredSize[1] == Catch::Approx(0.0f));
            REQUIRE(collapsed.Layout.FinalRect.Size[0] == Catch::Approx(0.0f));
            REQUIRE(collapsed.Layout.FinalRect.Size[1] == Catch::Approx(0.0f));

            // Visible child at BottomRight: (160, 80)
            RequireRect(visible.Layout.FinalRect, { 160.0f, 80.0f }, { 40.0f, 20.0f });
        }

        TEST_CASE("UserInterface::ArrangeLayoutNode::ArrangeLayoutNode nested Horizontal inside Vertical positions grandchildren correctly", "[arrange][nested]")
        {
            // root: Vertical, Fixed 200×200
            LayoutNode root{};
            root.Style.LayoutType = ELayoutType::Vertical;
            root.Style.ChildAlign = EAlignment::TopLeft;
            root.Style.WidthMode = ESizingMode::Fixed;
            root.Style.HeightMode = ESizingMode::Fixed;
            root.Style.FixedWidth = 200.0f;
            root.Style.FixedHeight = 200.0f;

            // row: Horizontal, Fixed width=200, Fixed height=50
            LayoutNode row{};
            row.Style.LayoutType = ELayoutType::Horizontal;
            row.Style.ChildAlign = EAlignment::TopLeft;
            row.Style.WidthMode = ESizingMode::Fixed;
            row.Style.HeightMode = ESizingMode::Fixed;
            row.Style.FixedWidth = 200.0f;
            row.Style.FixedHeight = 50.0f;

            LayoutNode gc1{}; gc1.Style.WidthMode = ESizingMode::Fixed; gc1.Style.HeightMode = ESizingMode::Fixed;
            gc1.Style.FixedWidth = 60.0f; gc1.Style.FixedHeight = 50.0f;

            LayoutNode gc2{}; gc2.Style.WidthMode = ESizingMode::Fixed; gc2.Style.HeightMode = ESizingMode::Fixed;
            gc2.Style.FixedWidth = 80.0f; gc2.Style.FixedHeight = 50.0f;

            row.PushBackChild(gc1);
            row.PushBackChild(gc2);

            // Node below the row
            LayoutNode below{};
            below.Style.WidthMode = ESizingMode::Fixed;
            below.Style.HeightMode = ESizingMode::Fixed;
            below.Style.FixedWidth = 100.0f;
            below.Style.FixedHeight = 30.0f;

            root.PushBackChild(row);
            root.PushBackChild(below);

            DoLayout(root, { 200.0f, 200.0f });

            // row at y=0 height=50, below at y=50
            REQUIRE(row.Layout.FinalRect.Origin[1] == Catch::Approx(0.0f));
            REQUIRE(below.Layout.FinalRect.Origin[1] == Catch::Approx(50.0f));

            // gc1 and gc2 inside row (world y=0)
            REQUIRE(gc1.Layout.FinalRect.Origin[0] == Catch::Approx(0.0f));
            REQUIRE(gc1.Layout.FinalRect.Origin[1] == Catch::Approx(0.0f));
            REQUIRE(gc2.Layout.FinalRect.Origin[0] == Catch::Approx(60.0f));
            REQUIRE(gc2.Layout.FinalRect.Origin[1] == Catch::Approx(0.0f));
        }

        TEST_CASE("UserInterface::ArrangeLinear::ArrangeLinear Horizontal child with Flex HeightMode fills parent height automatically", "[arrange][linear][flex]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Horizontal;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 200.0f;
            parent.Style.FixedHeight = 100.0f;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Fixed;
            child.Style.HeightMode = ESizingMode::Flex; // fills parent height in horizontal layout
            child.Style.FixedWidth = 50.0f;

            parent.PushBackChild(child);

            DoLayout(parent, { 200.0f, 100.0f });

            REQUIRE(child.Layout.FinalRect.Size[1] == Catch::Approx(100.0f));
        }

        TEST_CASE("UserInterface::ArrangeOverlay::ArrangeOverlay child with Percent sizing is positioned using fraction of available size", "[arrange][overlay][percent]")
        {
            LayoutNode parent{};
            parent.Style.LayoutType = ELayoutType::Overlay;
            parent.Style.ChildAlign = EAlignment::TopLeft;
            parent.Style.WidthMode = ESizingMode::Fixed;
            parent.Style.HeightMode = ESizingMode::Fixed;
            parent.Style.FixedWidth = 400.0f;
            parent.Style.FixedHeight = 200.0f;

            LayoutNode child{};
            child.Style.WidthMode = ESizingMode::Percent;
            child.Style.HeightMode = ESizingMode::Percent;
            child.Style.PercentWidth = 0.5f;
            child.Style.PercentHeight = 0.5f;

            parent.PushBackChild(child);

            DoLayout(parent, { 400.0f, 200.0f });

            // Child measured as 50% of available: 200×100, placed at TopLeft (0,0)
            RequireRect(child.Layout.FinalRect, { 0.0f, 0.0f }, { 200.0f, 100.0f });
        }
	}
}