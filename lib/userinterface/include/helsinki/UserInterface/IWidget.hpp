#pragma once

#include <helsinki/UserInterface/Layout.hpp>
#include <helsinki/UserInterface/DrawList.hpp>
#include <helsinki/UserInterface/InputEvent.hpp>

namespace hl
{
    class UiScene;

    class IWidget
    {
    public:
        virtual ~IWidget() = default;

        /** @brief Returns the unique identifier for this widget. */
        WidgetID GetID() const { return m_ID; }

        /** @brief Returns the layout identifier for this widget. */
        NodeID GetLayoutID() const { return m_LayoutID; }

        /** @brief Called immediately after the widget is constructed and associated with a layout node. */
        virtual void OnConstruct(UiScene& /*a_Scene*/) {}

        /** @brief Called immediately before the widget is destroyed and disassociated from its layout node. */
        virtual void OnDestroy(UiScene& /*a_Scene*/) {} ///< Called immediately before the widget is destroyed and disassociated from its layout node.

        /** @brief Called when the widget should render itself and its children. */
        virtual void OnPaint(UiScene& /*a_Scene*/, DrawList& /*a_DrawList*/) {}

        /** @brief Returns whether this widget can receive focus for input. */
        virtual bool IsFocusable(UiScene& /*a_Scene*/) const { return false; }

        /** @brief Called when this widget receives focus for input. */
        virtual void OnFocusReceived(UiScene& /*a_Scene*/) {}

        /** @brief Called when this widget loses focus for input. */
        virtual void OnFocusLost(UiScene& /*a_Scene*/) {}

        /** @brief Called when a pointer (e.g., mouse cursor) enters the widget's bounds. */
        virtual void OnPointerEnter(UiScene& /*a_Scene*/, const PointerEvent& /*a_Event*/) {}

        /** @brief Called when a pointer (e.g., mouse cursor) exits the widget's bounds. */
        virtual void OnPointerExit(UiScene& /*a_Scene*/, const PointerEvent& /*a_Event*/) {}

        /** @brief Called when an input button is pressed while this widget is focused. */
        virtual bool OnPressed(UiScene& /*a_Scene*/, const ButtonEvent& /*a_Event*/) { return false; }

        /** @brief Called when an input button is released while this widget is focused. */
        virtual bool OnReleased(UiScene& /*a_Scene*/, const ButtonEvent& /*a_Event*/) { return false; }

        void PaintChildren(UiScene& a_Scene, DrawList& a_DrawList) const;
    protected:
        friend UiScene;
        WidgetID m_ID{};
        NodeID m_LayoutID{};

    };
}