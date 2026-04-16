#pragma once

#include <memory>
#include <helsinki/System/Utils/Pool.hpp>
#include <helsinki/UserInterface/IWidget.hpp>
#include <helsinki/UserInterface/Navigation.hpp>

namespace hl
{

    using LayoutNodePool = Pool<LayoutNode>;
    using WidgetPool = Pool<std::unique_ptr<IWidget>>;

    /**
     * @brief Represents a UI scene containing a hierarchy of widgets and their associated layout nodes.
     * The UiScene class manages the lifecycle of widgets, processes input events, updates layout, and handles rendering.
     *
     * @example
     * // Example usage of the UiScene class:
     * UiScene scene;
     * DrawList drawList;
     *
     * // In your initialization code:
     * WidgetID root = scene.CreateRootWidget<RootWidget>();
     * WidgetID child1 = scene.CreateWidget<ButtonWidget>( root );
     * WidgetID child2 = scene.CreateWidget<TextWidget>( root );
     *
     * // In your main loop:
     * scene.ProcessInput( GetMousePosition(), IsMouseDown(), GetUIScale() );
     * scene.UpdateLayout( Vec2f{ 800.0f, 600.0f } );
     * scene.Render( drawList );
     */
    class UiScene
    {
    public:
        UiScene() = default;
        ~UiScene() = default;
        UiScene(const UiScene&) = delete;
        UiScene& operator=(const UiScene) = delete;
        UiScene(UiScene&&) = default;
        UiScene& operator=(UiScene&&) = default;

        LayoutNodePool Layouts{};     
        WidgetPool     Widgets{};
        WidgetID       RootWidget{};  ///< The WidgetID of the root widget in the scene, which serves as the entry point for layout and rendering.

        bool DispatchInputEvent(const InputEvent& a_Event);

        void UpdateLayout(Vec2f a_AvailableSize);

        void Render(DrawList& a_DrawList);

        // - Focus Management

        /** @brief Returns the WidgetID of the currently focused widget, or c_InvalidPoolID if no widget is focused. */
        WidgetID GetFocusedWidget() const { return m_FocusedWidget; }

        /** @brief Sets the focus to the specified widget, if it is focusable. */
        void SetFocus(WidgetID a_WidgetID);

        /** @brief Clears the focus from the current focused widget. */
        void ClearFocus() { SetFocus(c_InvalidPoolID); }

        // - Navigation

        void Navigate(ENavAction a_Action);

        void PushNavScope(WidgetID a_ScopeID);
        void PopNavScope();
        WidgetID GetCurrentNavScope() const { return m_NavStack.empty() ? RootWidget : m_NavStack.back().Scope; }

        // - Widget Management

        template<std::derived_from<IWidget> WidgetType, typename... Args>
        WidgetID CreateWidget(WidgetID a_ParentID, Args&&... a_Args);

        template<std::derived_from<IWidget> WidgetType, typename... Args>
        WidgetID CreateRootWidget(Args&&... a_Args);

        /** @brief Destroys the widget with the specified ID, including its children. */
        bool DestroyWidget(WidgetID a_WidgetID);

        HELSINKI_NODISCARD IWidget* GetWidget(WidgetID a_ID);

        HELSINKI_NODISCARD const IWidget* GetWidget(WidgetID a_ID) const;

        template<std::derived_from<IWidget> WidgetType>
        HELSINKI_NODISCARD WidgetType* GetWidget(WidgetID a_ID) { return dynamic_cast<WidgetType*>(GetWidget(a_ID)); }

        template<std::derived_from<IWidget> WidgetType>
        HELSINKI_NODISCARD const WidgetType* GetWidget(WidgetID a_ID) const { return dynamic_cast<const WidgetType*>(GetWidget(a_ID)); }

        template<std::invocable<IWidget&> Func>
        void ForEachChildWidget(WidgetID a_WidgetID, Func&& a_Func);

        template<std::invocable<const IWidget&> Func>
        void ForEachChildWidget(WidgetID a_WidgetID, Func&& a_Func) const;

        /** @brief Clears the scene */
        void Reset();

    protected:
        bool ProcessPointerEvent(const PointerEvent& a_Event);
        bool ProcessButtonEvent(const ButtonEvent& a_Event);
        WidgetID HitTest(WidgetID a_ID, Vec2f a_LogicalPos);

        WidgetID m_FocusedWidget{ c_InvalidPoolID };
        WidgetID m_HoveredWidget{ c_InvalidPoolID };
        PointerEvent m_LastPointerEvent{}; ///< The last pointer event received, used for hit testing and hover state management.

        struct NavScope
        {
            WidgetID Scope{ c_InvalidPoolID };    ///< The container widget
            WidgetID Restored{ c_InvalidPoolID }; ///< The widget to restore on pop
        };
        std::vector<NavScope> m_NavStack{}; ///< Stack of navigation scopes used to manage focus during keyboard/gamepad navigation, allowing for nested navigation contexts.

    };

    template<std::derived_from<IWidget> WidgetType, typename... Args>
    WidgetID UiScene::CreateRootWidget(Args&&... a_Args)
    {
        WidgetID id = CreateWidget<WidgetType>(c_InvalidPoolID, std::forward<Args>(a_Args)...);
        RootWidget = id;
        return id;
    }

    template<std::derived_from<IWidget> WidgetType, typename... Args>
    WidgetID UiScene::CreateWidget(WidgetID a_ParentID, Args&&... a_Args)
    {
        std::unique_ptr<IWidget> widgetPtr = std::make_unique<WidgetType>(std::forward<Args>(a_Args)...);
        IWidget* widget = widgetPtr.get();

        // Allocate layout node and widget
        NodeID     nodeID = Layouts.Allocate();
        WidgetID   widgetID = Widgets.Allocate(std::move(widgetPtr));

        LayoutNode* node = Layouts.Get(nodeID);

        // Wire widget <-> node
        widget->m_ID = widgetID;
        widget->m_LayoutID = nodeID;
        node->WidgetID = widgetID;

        if (a_ParentID != c_InvalidPoolID)
        {
            if (IWidget* parentWidget = GetWidget(a_ParentID))
            {
                if (LayoutNode* parentNode = Layouts.Get(parentWidget->GetLayoutID()))
                    parentNode->PushBackChild(*node);
            }
        }

        // Call construct after fully initialized and linked into hierarchy, in case widget logic depends on that
        widget->OnConstruct(*this);

        return widgetID;
    }

    template<std::invocable<IWidget&> Func>
    void UiScene::ForEachChildWidget(WidgetID a_WidgetID, Func&& a_Func)
    {
        IWidget* widget = GetWidget(a_WidgetID);
        if (!widget) return;

        LayoutNode* node = Layouts.Get(widget->GetLayoutID());
        if (!node) return;

        node->ForEachChild([&](LayoutNode& childNode)
            {
                IWidget* childWidget = GetWidget(childNode.WidgetID);
                if (childWidget) a_Func(*childWidget);
            });
    }

    template<std::invocable<const IWidget&> Func>
    void UiScene::ForEachChildWidget(WidgetID a_WidgetID, Func&& a_Func) const
    {
        const IWidget* widget = GetWidget(a_WidgetID);
        if (!widget) return;

        const LayoutNode* node = Layouts.Get(widget->GetLayoutID());
        if (!node) return;

        node->ForEachChild([&](const LayoutNode& childNode)
            {
                const IWidget* childWidget = GetWidget(childNode.WidgetID);
                if (childWidget) a_Func(*childWidget);
            });
    }

}