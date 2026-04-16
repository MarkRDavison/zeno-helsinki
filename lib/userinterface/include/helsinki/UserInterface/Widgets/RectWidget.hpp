#pragma once

#include <helsinki/UserInterface/LayoutEngine.hpp>
#include <helsinki/UserInterface/IWidget.hpp>

namespace hl
{
    class UiScene;
    class RectWidget : public IWidget
    {
    public:
        RectWidget(Vec4f a_Color, std::string a_Name, CornerRounding a_Rounding = CornerRounding::Uniform(10))
            : Color(a_Color)
            , Name(a_Name)
            , Rounding(a_Rounding)
        {
        }

        std::string Name;
        Vec4f Color;
        CornerRounding Rounding;

        void OnPaint(UiScene& a_Scene, DrawList& a_DrawList) override;

        bool IsFocusable(UiScene& /*a_Scene*/) const override
        {
            return true;
        }
    };
}