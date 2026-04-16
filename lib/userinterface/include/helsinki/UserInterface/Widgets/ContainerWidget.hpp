#pragma once

#include <helsinki/UserInterface/IWidget.hpp>

namespace hl
{
    class ContainerWidget : public IWidget
    {
    public:
        void OnPaint(UiScene& a_Scene, DrawList& a_DrawList) override
        {
            PaintChildren(a_Scene, a_DrawList);
        }

        bool IsFocusable(UiScene& /*a_Scene*/) const override
        {
            return false;
        }
    };
}