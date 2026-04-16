#pragma once

#include <helsinki/UserInterface/Layout.hpp>

namespace hl
{
    // Forward declarations of layout functions
    Vec2f MeasureLayoutNode(LayoutNode& a_Node, Vec2f a_AvailableSize);
    void  ArrangeLayoutNode(LayoutNode& a_Node, Rectf a_AllocatedRect);

    void  ArrangeAnchored(LayoutNode& a_Node, Rectf a_Container);
    void  ArrangeOverlay(LayoutNode& a_Node, Rectf a_Inner);
    void  ArrangeLinear(LayoutNode& a_Node, Rectf a_Inner);

    EAlignment ResolveAlign(const LayoutNode& a_Child, const LayoutNode& a_Parent);
    Rectf AlignRect(Vec2f a_ContentSize, Rectf a_Container, EAlignment a_Align);
    f32   AlignCrossAxis(f32 a_ChildSize, f32 a_ParentPos, f32 a_ParentSize, EAlignment a_Align, bool a_IsMainAxisHorizontal);

}