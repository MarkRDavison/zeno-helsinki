#pragma once

#include <helsinki/UserInterface/IWidget.hpp>

namespace hl
{
    class ContainerWidget : public IWidget
    {
    public:
        ContainerWidget()
        {
        }

        void OnPaint(UiScene& a_Scene, DrawList& a_DrawList) override
        {
            a_Scene.ForEachChildWidget(GetID(), [&](IWidget& child)
                {
                    child.OnPaint(a_Scene, a_DrawList);
                });
        }

        bool IsFocusable(UiScene& /*a_Scene*/) const override
        {
            return false;
        }
    };
}