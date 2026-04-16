#pragma once

#include <vector>
#include <cassert>
#include <variant>
#include <helsinki/System/Types.hpp>

namespace hl
{

    struct CornerRounding
    {
        f32 TopLeft{ 0.f };
        f32 TopRight{ 0.f };
        f32 BottomLeft{ 0.f };
        f32 BottomRight{ 0.f };

        static constexpr CornerRounding None() { return {}; }
        static constexpr CornerRounding Uniform(f32 a_Radius) { return { a_Radius, a_Radius, a_Radius, a_Radius }; }
        static constexpr CornerRounding Symmetric(f32 a_Top, f32 a_Bottom) { return { a_Top, a_Top, a_Bottom, a_Bottom }; }

        constexpr CornerRounding operator+(f32 a_Amount) const
        {
            return {
                .TopLeft = TopLeft + a_Amount,
                .TopRight = TopRight + a_Amount,
                .BottomLeft = BottomLeft + a_Amount,
                .BottomRight = BottomRight + a_Amount
            };
        }
    };

    using CustomDrawFunc = void(*)(const struct DrawCmd& _cmd);

    struct DrawCmd
    {
        struct RectCmd
        {
            Vec4f Color;
            Rectf Rect;
            CornerRounding Rounding;
        };

        struct RectBorderCmd
        {
            Vec4f Color;
            Rectf Rect;
            CornerRounding Rounding;
            f32 Thickness;
        };

        struct CircleCmd
        {
            Vec4f Color;
            Vec2f Center;
            f32 Radius;
        };

        struct CircleBorderCmd
        {
            Vec4f Color;
            Vec2f Center;
            f32 Radius;
            f32 Thickness;
        };

        struct CustomCmd { CustomDrawFunc Func; void* UserData; };

        Mat3f Transform{ c_Identity<Mat3f> };
        Rectf ClipRect;

        std::variant<
            RectCmd,
            RectBorderCmd,
            CircleCmd,
            CircleBorderCmd,
            CustomCmd
        > Payload;
    };

    struct DrawList
    {
        std::vector<DrawCmd> Commands;     ///< Buffered draw commands to be executed by the renderer.
        std::vector<Rectf> ClipStack;      ///< Stack of clipping rectangles. The current clipping rectangle is the intersection of all rectangles in the stack.
        std::vector<Mat3f> TransformStack; ///< Stack of transformation matrices. The current transformation is the product of all matrices in the stack.

        void Clear()
        {
            Commands.clear();
            ClipStack.clear();
            TransformStack.clear();
        }

        const Mat3f& CurrentTransform() const
        {
            return TransformStack.empty() ? c_Identity<Mat3f> : TransformStack.back();
        }

        DrawList& PushTransform(const Mat3f& a_Transform)
        {
            TransformStack.push_back(CurrentTransform() * a_Transform);
            return *this;
        }
        
        DrawList& PopTransform()
        {
            assert(!TransformStack.empty() && "Called PopTransform too many times: no transform to pop.");
            TransformStack.pop_back();
            return *this;
        }

        Rectf CurrentClipRect() const
        {
            if (ClipStack.empty())
                return Rectf::Infinite();

            return ClipStack.back();
        }

        DrawList& PushClipRect(Rectf a_Rect)
        {
            // Intersect with current clip rect if one exists
            if (!ClipStack.empty())
            {
                a_Rect = a_Rect.Intersection(ClipStack.back());
            }

            ClipStack.push_back(a_Rect);
            return *this;
        }

        DrawList& PopClipRect()
        {
            assert(!ClipStack.empty() && "Called PopClipRect too many times: no clip rect to pop.");
            ClipStack.pop_back();
            return *this;
        }

        DrawList& AddRect(Vec4f a_Color, Rectf a_Rect, CornerRounding a_Rounding = {})
        {
            Commands.push_back(DrawCmd{
                .Transform = CurrentTransform(),
                .ClipRect = CurrentClipRect(),
                .Payload = DrawCmd::RectCmd{.Color = a_Color, .Rect = a_Rect, .Rounding = a_Rounding }
                });
            return *this;
        }

        DrawList& AddRectBorder(Vec4f a_Color, Rectf a_Rect, CornerRounding a_Rounding = {}, f32 a_Thickness = 1.f)
        {
            Commands.push_back(DrawCmd{
                .Transform = CurrentTransform(),
                .ClipRect = CurrentClipRect(),
                .Payload = DrawCmd::RectBorderCmd{.Color = a_Color, .Rect = a_Rect, .Rounding = a_Rounding, .Thickness = a_Thickness }
                });
            return *this;
        }

        DrawList& AddCircle(Vec4f a_Color, Vec2f a_Center, f32 a_Radius)
        {
            Commands.push_back(DrawCmd{
                .Transform = CurrentTransform(),
                .ClipRect = CurrentClipRect(),
                .Payload = DrawCmd::CircleCmd{.Color = a_Color, .Center = a_Center, .Radius = a_Radius }
                });
            return *this;
        }

        DrawList& AddCircleBorder(Vec4f a_Color, Vec2f a_Center, f32 a_Radius, f32 a_Thickness = 1.f)
        {
            Commands.push_back(DrawCmd{
                .Transform = CurrentTransform(),
                .ClipRect = CurrentClipRect(),
                .Payload = DrawCmd::CircleBorderCmd{.Color = a_Color, .Center = a_Center, .Radius = a_Radius, .Thickness = a_Thickness }
                });
            return *this;
        }

        DrawList& AddCustom(CustomDrawFunc a_Func, void* a_UserData = nullptr)
        {
            assert(a_Func && "Custom draw command requires a valid function pointer.");

            if (a_Func)
            {
                Commands.push_back(DrawCmd{
                    .Transform = CurrentTransform(),
                    .ClipRect = CurrentClipRect(),
                    .Payload = DrawCmd::CustomCmd{.Func = a_Func, .UserData = a_UserData }
                    });
            }

            return *this;
        }
    };
}