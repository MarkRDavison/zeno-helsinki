#pragma once

#include <helsinki/System/Types.hpp>
#include <helsinki/System/Utils/Pool.hpp>
#include <cassert>

namespace hl
{
    using WidgetID = PoolID;
    using NodeID = PoolID;

    enum EAlignment : std::uint8_t
    {
        Inherit = 0,

        Left = 1 << 0,
        HCenter = 1 << 1,
        Right = 1 << 2,

        Top = 1 << 3,
        VCenter = 1 << 4,
        Bottom = 1 << 5,

        TopLeft = Top | Left,
        TopCenter = Top | HCenter,
        TopRight = Top | Right,

        CenterLeft = VCenter | Left,
        Center = VCenter | HCenter,
        CenterRight = VCenter | Right,

        BottomLeft = Bottom | Left,
        BottomCenter = Bottom | HCenter,
        BottomRight = Bottom | Right,

        HStretch = Left | Right,
        VStretch = Top | Bottom,
        StretchFill = HStretch | VStretch,
    };

    enum class EPositioningMode : std::uint8_t
    {
        Flow,
        Anchored,
    };

    enum class ESizingMode : std::uint8_t
    {
        Content,
        Fixed,
        Percent,
        Flex,
    };

    enum class EWrapMode : std::uint8_t
    {
        NoWrap,
        Wrap
    };

    enum class ELayoutType : std::uint8_t
    {
        Horizontal,
        Vertical,
        Overlay,
        Grid
    };

    struct Anchor
    {
        Vec2f Min{ 0.0f, 0.0f };
        Vec2f Max{ 0.0f, 0.0f };
        Vec2f Pivot{ 0.0f, 0.0f };
        Vec2f Offset{ 0.0f, 0.0f };

        static constexpr Anchor TopLeft() { return { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } }; }
        static constexpr Anchor TopCenter() { return { { 0.5f, 0.0f }, { 0.5f, 0.0f }, { 0.5f, 0.0f } }; }
        static constexpr Anchor TopRight() { return { { 1.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f } }; }
        static constexpr Anchor CenterLeft() { return { { 0.0f, 0.5f }, { 0.0f, 0.5f }, { 0.0f, 0.5f } }; }
        static constexpr Anchor Center() { return { { 0.5f, 0.5f }, { 0.5f, 0.5f }, { 0.5f, 0.5f } }; }
        static constexpr Anchor CenterRight() { return { { 1.0f, 0.5f }, { 1.0f, 0.5f }, { 1.0f, 0.5f } }; }
        static constexpr Anchor BottomLeft() { return { { 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 1.0f } }; }
        static constexpr Anchor BottomCenter() { return { { 0.5f, 1.0f }, { 0.5f, 1.0f }, { 0.5f, 1.0f } }; }
        static constexpr Anchor BottomRight() { return { { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f } }; }

        static constexpr Anchor StretchAll() { return { { 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.5f, 0.5f } }; }
        static constexpr Anchor StretchTop() { return { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 0.5f, 0.0f } }; }
        static constexpr Anchor StretchBottom() { return { { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.5f, 1.0f } }; }
        static constexpr Anchor StretchLeft() { return { { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.5f } }; }
        static constexpr Anchor StretchRight() { return { { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.5f } }; }
    };

    struct Visibility
    {
        enum EType : std::uint8_t
        {
            Visible,
            Hidden,
            Collapsed,
        };

        EType Value{ Visible };

        constexpr bool AffectsLayout() const { return Value != Collapsed; }

        constexpr bool IsRendered() const { return Value == Visible; }

        constexpr bool IsHitTestable() const { return Value == Visible; }
    };

    struct Edges
    {
        f32 Top;
        f32 Right;
        f32 Bottom;
        f32 Left;

        /** @brief Calculates the total horizontal inset. */
        constexpr f32 Horizontal() const { return Left + Right; }

        /** @brief Calculates the total vertical inset. */
        constexpr f32 Vertical() const { return Top + Bottom; }

        /** @brief Calculates the total inset as a vector. */
        constexpr Vec2f Total() const { return { Horizontal(), Vertical() }; }

        /** @brief Applies the edge insets to a given rectangle, returning a new rectangle that is reduced by the specified insets. */
        constexpr Rectf Apply(Rectf a_Rect) const
        {
            return a_Rect.FromMinMax(a_Rect.Min() + Vec2f{ Left, Top },
                a_Rect.Max() - Vec2f{ Right, Bottom });
        }

        /** @brief Combines two Edges by adding their respective values together. */
        constexpr Edges operator+(const Edges& a_Other) const
        {
            return { Top + a_Other.Top, Right + a_Other.Right, Bottom + a_Other.Bottom, Left + a_Other.Left };
        }

        /** @brief Scales the edge insets by a scalar value, multiplying each edge by the specified factor. */
        constexpr Edges operator*(f32 a_Scalar) const
        {
            return { Top * a_Scalar, Right * a_Scalar, Bottom * a_Scalar, Left * a_Scalar };
        }

        /** @brief Scales the edge insets by a scalar value, dividing each edge by the specified factor. */
        constexpr Edges operator/(f32 a_Scalar) const
        {
            return { Top / a_Scalar, Right / a_Scalar, Bottom / a_Scalar, Left / a_Scalar };
        }

        /** @brief Initializes all edges to the same value. */
        constexpr Edges(f32 a_UniformValue = 0.0f) : Top(a_UniformValue), Right(a_UniformValue), Bottom(a_UniformValue), Left(a_UniformValue) {}

        /** @brief Initializes horizontal and vertical edges separately. */
        constexpr Edges(f32 a_Horizontal, f32 a_Vertical) : Top(a_Vertical), Right(a_Horizontal), Bottom(a_Vertical), Left(a_Horizontal) {}

        /** @brief Initializes each edge individually. */
        constexpr Edges(f32 a_Top, f32 a_Right, f32 a_Bottom, f32 a_Left) : Top(a_Top), Right(a_Right), Bottom(a_Bottom), Left(a_Left) {}

        /** @brief Initializes all edges to the same value. */
        static constexpr Edges Uniform(f32 a_Value) { return { a_Value }; }

        /** @brief Initializes horizontal and vertical edges separately. */
        static constexpr Edges Symmetric(f32 a_Horizontal, f32 a_Vertical) { return { a_Horizontal, a_Vertical }; }

        /** @brief Initializes each edge individually. */
        static constexpr Edges Asymmetric(f32 a_Top, f32 a_Right, f32 a_Bottom, f32 a_Left) { return { a_Top, a_Right, a_Bottom, a_Left }; }
    };

    struct Constraints
    {
        Vec2f MinSize{ 0.0f, 0.0f };
        Vec2f MaxSize{ std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max() };

        /** @brief Creates unbounded constraints (i.e., no minimum or maximum size std::numeric_limits). */
        static constexpr Constraints Unbounded() { return {}; }

        /** @brief Creates fixed size constraints. */
        static constexpr Constraints Fixed(Vec2f a_Size) { return { a_Size, a_Size }; }

        /** @brief Creates minimum size constraints with no maximum limit. */
        static constexpr Constraints AtLeast(Vec2f a_Min) { return { a_Min, { std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max() } }; }

        /** @brief Creates maximum size constraints with no minimum limit. */
        static constexpr Constraints AtMost(Vec2f a_Max) { return { { 0.0f, 0.0f }, a_Max }; }
    };

    struct LayoutStyle
    {

        // - Layout properties
        f32         Spacing{ 0.0f };                    ///< The spacing to apply between child elements in a container, in pixels.
        ELayoutType LayoutType{ ELayoutType::Overlay }; ///< The layout type to use for arranging child elements (if this element is a container).
        EAlignment  ChildAlign{ EAlignment::TopLeft };  ///< Default alignment for child elements within this container.
        EWrapMode   WrapMode{ EWrapMode::NoWrap };      ///< The wrap mode to use when child elements exceed the available space in a container.
        bool        IsFocusScope{ false };

        // - Positioning properties
        Edges Padding{};        ///< The padding to apply around the content of the element, in pixels.
        Edges Margin{};         ///< The margin to apply around the element itself, in pixels.
        struct Anchor Anchor {}; ///< The anchor points for the element, used when PositionMode is set to Anchored.
        EPositioningMode PositionMode{ EPositioningMode::Flow }; ///< The positioning mode for the element, determining how it is positioned relative to its parent container.

        // - Alignment properties
        EAlignment SelfAlign{ EAlignment::Inherit }; ///< Overrides parent's ChildAlign for this element. Only applicable when PositionMode is Flow.

        // - Sizing properties
        ESizingMode WidthMode{ ESizingMode::Content };  ///< The sizing mode for the width of the element.
        ESizingMode HeightMode{ ESizingMode::Content }; ///< The sizing mode for the height of the element.
        f32 FixedWidth{ 0.0f };  ///< The fixed width to use when WidthMode is set to Fixed.
        f32 FixedHeight{ 0.0f }; ///< The fixed height to use when HeightMode is set to Fixed.

        /**
         * @note PercentWidth/PercentHeight are only meaningful when the parent axis is Fixed or Flex.
         * Inside a Content-sized parent, Flex children fall back to Content (zero intrinsic size) and PercentWidth/Height is ignored.
         */
        f32 PercentWidth{ 0.0f };      ///< The percentage of the available width to use when WidthMode is set to Fill.
        f32 PercentHeight{ 0.0f };     ///< The percentage of the available height to use when HeightMode is set to Fill.

        f32 FlexGrow{ 0.0f };          ///< Determines how much of the remaining space the element should occupy relative to its siblings.
        Constraints SizeConstraints{}; ///< The size constraints to consider when laying out the element.
        bool StopsHitTests{ false };
    };

    struct LayoutResult
    {
        Rectf FinalRect{};    ///< The final position and size of the element after layout, in absolute coordinates. Filled by the Arrange step.
        Vec2f DesiredSize{};  ///< The desired size of the element based on its content and constraints. Filled by the Measure step.
        bool IsDirty{ true }; ///< Whether the layout needs to be recalculated. Set to true when properties affecting layout are changed.
        struct Visibility Visibility {}; ///< The visibility state of the element, which can affect both rendering and layout.
    };

    struct LayoutNode
    {
        LayoutStyle Style{};
        LayoutResult Layout{};
        u32 NumChildren{ 0 };
        LayoutNode* Parent{ nullptr };
        LayoutNode* FirstChild{ nullptr };
        LayoutNode* LastChild{ nullptr };
        LayoutNode* PrevSibling{ nullptr };
        LayoutNode* NextSibling{ nullptr };

        WidgetID WidgetID;

        void DetachFromParent();

        void PushBackChild(LayoutNode& a_Child);

        void PushFrontChild(LayoutNode& a_Child);

        void InsertChildAfter(LayoutNode& a_Child, LayoutNode& a_Sibling);

        void InsertChildBefore(LayoutNode& a_Child, LayoutNode& a_Sibling);

        template<std::invocable<LayoutNode&> Func>
        void ForEachChild(Func&& a_Func);

        template<std::invocable<const LayoutNode&> Func>
        void ForEachChild(Func&& a_Func) const;

        template<std::invocable<LayoutNode&> Func>
        void ForEachDescendant(Func&& a_Func);
    };

    // === Inline Implementations ===

    template<std::invocable<LayoutNode&> Func>
    void LayoutNode::ForEachChild(Func&& a_Func)
    {
        for (LayoutNode* child = FirstChild; child != nullptr; child = child->NextSibling)
        {
            std::forward<Func>(a_Func)(*child);
        }
    }

    template<std::invocable<const LayoutNode&> Func>
    void LayoutNode::ForEachChild(Func&& a_Func) const
    {
        for (const LayoutNode* child = FirstChild; child != nullptr; child = child->NextSibling)
        {
            std::forward<Func>(a_Func)(*child);
        }
    }

    template<std::invocable<LayoutNode&> Func>
    void LayoutNode::ForEachDescendant(Func&& a_Func)
    {
        ForEachChild([&](LayoutNode& child)
            {
                std::forward<Func>(a_Func)(child);
                child.ForEachDescendant(std::forward<Func>(a_Func));
            });
    }
}