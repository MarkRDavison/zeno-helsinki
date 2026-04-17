#include <helsinki/UserInterface/Widgets/ButtonBaseWidget.hpp>
#include <helsinki/UserInterface/UiScene.hpp>

namespace hl
{
    void ButtonBaseWidget::OnPaint(UiScene& a_Scene, DrawList& a_DrawList)
    {
        const LayoutNode* node = a_Scene.Layouts.Get(GetLayoutID());
        if (!node)
        {
            return;
        }

        const Rectf& rect = node->Layout.FinalRect;

        if (a_Scene.GetFocusedWidget() == GetID())
        {
            a_DrawList.AddRect(Vec4f(1.0f, 1.0f, 1.0f, 1.0f), rect.Expanded(4.f), Rounding + 4);
        }

        auto col = Color;

        if (_pressStarted)
        {
            col = Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
        }
        else if (_hovered)
        {
            col = Vec4f(0.0f, 1.0f, 0.0f, 1.0f);
        }

        a_DrawList.AddRect(col, rect, Rounding);

        a_DrawList.PushClipRect(rect);
        
        PaintChildren(a_Scene, a_DrawList);

        a_DrawList.PopClipRect();
    }
}