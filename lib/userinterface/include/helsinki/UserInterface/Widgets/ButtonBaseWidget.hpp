#pragma once

#include <helsinki/UserInterface/IWidget.hpp>
#include <iostream>

namespace hl
{
    class ButtonBaseWidget : public IWidget
    {
    public:

        void OnPaint(UiScene& a_Scene, DrawList& a_DrawList) override
        {
            PaintChildren(a_Scene, a_DrawList);
        }

        /** @brief Called when an input button is released while this widget is focused. */
        bool OnReleased(UiScene& /*a_Scene*/, const ButtonEvent& /*a_Event*/) override
        {
            std::cout << "ButtonBaseWidget::OnReleased" << std::endl;
            return false;
        }
        
        void OnPointerEnter(UiScene& /*a_Scene*/, const PointerEvent& /*a_Event*/) override
        {
            std::cout << "ButtonBaseWidget::OnpointerEnter" << std::endl;
        }

        bool IsFocusable(UiScene& /*a_Scene*/) const override
        {
            return true;
        }
    };
}