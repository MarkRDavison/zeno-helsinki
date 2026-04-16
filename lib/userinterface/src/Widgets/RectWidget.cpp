#include <helsinki/UserInterface/Widgets/RectWidget.hpp>
#include <helsinki/UserInterface/UiScene.hpp>

namespace hl
{

    void RectWidget::OnPaint(UiScene& a_Scene, DrawList& a_DrawList)
    {
        const LayoutNode* node = a_Scene.Layouts.Get(GetLayoutID());
        if (!node)
        {
            return;
        }

        const Rectf& rect = node->Layout.FinalRect;

        //RenderTransform transform{};
        //transform.Angle = Radians{ Degreesf{ angle } };
        //transform.Scale = Vec2f{ 0.75,0.75 } + Vec2f{ 1.f + 0.5f * std::sin(time * 0.5f), 1.f + 0.5f * std::sin(time * 0.5f) } * 0.1f; // Scale between 0.5 and 1.5
        //
        //a_DrawList.PushTransform( transform.ToMatrix( rect ) );

        if (a_Scene.GetFocusedWidget() == GetID())
        {
            a_DrawList.AddRect(Vec4f(1.0f, 1.0f, 1.0f, 1.0f), rect.Expanded(4.f), Rounding + 4);
        }

        a_DrawList.AddRect(Color, rect, Rounding);

        a_DrawList.PushClipRect(rect);
        a_Scene.ForEachChildWidget(GetID(), [&](IWidget& child)
            {
                child.OnPaint(a_Scene, a_DrawList);
            });
        a_DrawList.PopClipRect();

        //a_DrawList.PopTransform();
    }
}