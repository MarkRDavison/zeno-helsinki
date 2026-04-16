#pragma once

#include <helsinki/UserInterface/IWidget.hpp>
#include <functional>
#include <iostream>

namespace hl
{
    class ButtonBaseWidget : public IWidget
    {
    public:
        ButtonBaseWidget(Vec4f a_Color, CornerRounding a_Rounding = CornerRounding::Uniform(10))
            : Color(a_Color)
            , Rounding(a_Rounding)
        {
        }

        Vec4f Color;
        CornerRounding Rounding;

        void OnPaint(UiScene& a_Scene, DrawList& a_DrawList) override;
        virtual void PaintContent(UiScene& /*a_Scene*/, DrawList& /*a_DrawList*/) {};

        bool OnPressed(UiScene& /*a_Scene*/, const ButtonEvent& /*a_Event*/) override
        {
            _pressStarted = true;
            return true;
        }

        bool OnReleased(UiScene& /*a_Scene*/, const ButtonEvent& /*a_Event*/) override
        {
            if (_pressStarted)
            {
                OnClick();
                _pressStarted = false;
                return true;
            }

            return false;
        }

        void OnPointerEnter(UiScene& /*a_Scene*/, const PointerEvent& /*a_Event*/) override
        {
            _hovered = true;
        }

        void OnPointerExit(UiScene& /*a_Scene*/, const PointerEvent& /*a_Event*/) override
        {
            _pressStarted = false;
            _hovered = false;
        }

        bool IsFocusable(UiScene& /*a_Scene*/) const override
        {
            return true;
        }

        std::function<void()> OnClick;

    private:
        bool _pressStarted{ false };
        bool _hovered{ false };
    };
}