#include <helsinki/UserInterface/IWidget.hpp>
#include <helsinki/UserInterface/UiScene.hpp>

namespace hl
{

    void IWidget::PaintChildren(UiScene& a_Scene, DrawList& a_DrawList) const
    {
        a_Scene.ForEachChildWidget(GetID(), [&](IWidget& child)
            {
                child.OnPaint(a_Scene, a_DrawList);
            });
    }
}