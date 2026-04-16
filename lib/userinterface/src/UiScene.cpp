#include <helsinki/UserInterface/UiScene.hpp>
#include <helsinki/UserInterface/LayoutEngine.hpp>

namespace hl
{

    bool UiScene::DispatchInputEvent(const InputEvent& a_Event)
    {
        
        if (Holds<PointerEvent>(a_Event.Payload))
        {
            return ProcessPointerEvent(Get<PointerEvent>(a_Event.Payload));
        }

        if (Holds<ButtonEvent>(a_Event.Payload))
        {
            return ProcessButtonEvent(Get<ButtonEvent>(a_Event.Payload));
        }
        
        return false; // Event type not handled
    }

    void UiScene::UpdateLayout(Vec2f a_AvailableSize)
    {
        if (IWidget* root = GetWidget(RootWidget))
        {
            if (LayoutNode* rootNode = Layouts.Get(root->GetLayoutID()))
            {
                MeasureLayoutNode(*rootNode, a_AvailableSize);
                ArrangeLayoutNode(*rootNode, Rectf{ Vec2f{ 0.f, 0.f }, a_AvailableSize });
            }
        }

        // Re-run hit test to update hovered widget based on new layout
        // TODO: This is a bit unclean and potentially incorrect
        ProcessPointerEvent(m_LastPointerEvent);
    }

    void UiScene::Render(DrawList& a_DrawList)
    {
        if (IWidget* root = GetWidget(RootWidget))
        {
            root->OnPaint(*this, a_DrawList);
        }
    }

    void UiScene::SetFocus(WidgetID a_WidgetID)
    {
        if (IWidget* currentFocus = GetWidget(m_FocusedWidget))
        {
            if (currentFocus->GetID() == a_WidgetID)
                return; // Already focused

            currentFocus->OnFocusLost(*this);
        }

        m_FocusedWidget = a_WidgetID;
        if (IWidget* newFocus = GetWidget(m_FocusedWidget))
            newFocus->OnFocusReceived(*this);
    }

    void UiScene::Navigate(ENavAction a_Action)
    {
        const auto FocusFirstIn = [&](WidgetID a_ScopeID)
            {
                IWidget* scopeWidget = GetWidget(a_ScopeID);
                if (!scopeWidget) return;

                LayoutNode* scopeNode = Layouts.Get(scopeWidget->GetLayoutID());
                if (!scopeNode) return;

                // Walk children in layout order, find first focusable
                for (LayoutNode* child = scopeNode->FirstChild; child; child = child->NextSibling)
                {
                    IWidget* w = GetWidget(child->WidgetID);
                    if (w && w->IsFocusable(*this))
                    {
                        SetFocus(child->WidgetID);
                        return;
                    }
                }
            };

        if (a_Action == ENavAction::Cancel)
        {
            PopNavScope();
            return;
        }

        if (a_Action == ENavAction::Activate)
        {
            // If focused widget is itself a scope, enter it
            WidgetID focused = GetFocusedWidget();
            if (IWidget* w = GetWidget(focused))
            {
                LayoutNode* node = Layouts.Get(w->GetLayoutID());
                if (node && node->Style.IsFocusScope)
                {
                    PushNavScope(focused);
                    // Auto-focus first focusable child in new scope
                    FocusFirstIn(focused);
                    return;
                }
            }
            // Otherwise activate the leaf
            if (IWidget* w = GetWidget(focused))
                w->OnPressed(*this, ButtonEvent{ .Button = EButtonID::KeyEnter, .Pressed = true });
            return;
        }

        // Directional nav within current scope
        WidgetID scopeID = GetCurrentNavScope();
        WidgetID focused = GetFocusedWidget();

        IWidget* scopeWidget = GetWidget(scopeID);
        LayoutNode* scopeNode = scopeWidget ? Layouts.Get(scopeWidget->GetLayoutID()) : nullptr;
        IWidget* focusedWidget = GetWidget(focused);
        LayoutNode* focusedNode = focusedWidget ? Layouts.Get(focusedWidget->GetLayoutID()) : nullptr;

        if (!scopeNode) return;

        if (!focusedNode)
        {
            FocusFirstIn(scopeID);
            return;
        }

        auto currentChild = scopeNode->FirstChild;

        std::vector<LayoutNode*> focusableNodes{};

        while (true)
        {
            if (currentChild != nullptr)
            {
                auto w = GetWidget(currentChild->WidgetID);

                if (w != nullptr && (w->IsFocusable(*this) || currentChild->Style.IsFocusScope))
                {
                    focusableNodes.push_back(currentChild);
                }
            }

            if (currentChild == scopeNode->LastChild)
            {
                break;
            }

            currentChild = currentChild->NextSibling;
        }

        const LayoutNode* nextNode = FindNavigatableNode(a_Action, focusedNode, focusableNodes);
        
        if (nextNode)
        {
            SetFocus(nextNode->WidgetID);
        }

        // If no candidate found, optionally pop scope (navigated off the edge)
        // else PopScope();

    }

    void UiScene::PushNavScope(WidgetID a_ScopeID)
    {
        m_NavStack.push_back(NavScope{
            .Scope = a_ScopeID,
            .Restored = GetFocusedWidget()
            });
    }

    void UiScene::PopNavScope()
    {
        ClearFocus();

        if (m_NavStack.empty())
            return; // No scopes to pop, already at root

        WidgetID restored = m_NavStack.back().Restored;
        m_NavStack.pop_back();
        if (restored != c_InvalidPoolID)
            SetFocus(restored);
    }

    IWidget* UiScene::GetWidget(WidgetID a_ID)
    {
        if (std::unique_ptr<IWidget>* widget = Widgets.Get(a_ID))
            return widget->get();

        return nullptr;
    }

    const IWidget* UiScene::GetWidget(WidgetID a_ID) const
    {
        if (const std::unique_ptr<IWidget>* widget = Widgets.Get(a_ID))
            return widget->get();

        return nullptr;
    }

    bool UiScene::DestroyWidget(WidgetID a_WidgetID)
    {
        IWidget* widget = GetWidget(a_WidgetID);
        if (!widget)
            return false; // Widget not found

        LayoutNode* node = Layouts.Get(widget->GetLayoutID());
        if (!node)
            return false; // Layout node not found

        // Recursively destroy child widgets
        node->ForEachChild([&](LayoutNode& childNode)
            {
                DestroyWidget(childNode.WidgetID);
            });

        widget->OnDestroy(*this); // TODO: Does it make more sense to call this before or after destroying children?

        // Deallocate widget and layout node
        Widgets.Deallocate(widget->GetID());
        Layouts.Deallocate(widget->GetLayoutID());

        return true;
    }

    void UiScene::Reset()
    {
        Layouts.Clear();
        Widgets.Clear();
        RootWidget = c_InvalidPoolID;
        m_HoveredWidget = c_InvalidPoolID;
        ClearFocus();
        m_NavStack.clear();
    }

    WidgetID UiScene::HitTest(WidgetID a_ID, Vec2f a_LogicalPos)
    {
        IWidget* widget = GetWidget(a_ID);
        if (!widget) return c_InvalidPoolID;

        LayoutNode* node = Layouts.Get(widget->GetLayoutID());
        if (!node || !node->Layout.Visibility.IsHitTestable()) return c_InvalidPoolID;

        if (!node->Layout.FinalRect.Contains(a_LogicalPos)) return c_InvalidPoolID;

        // Check children first (front-to-back, last child wins)
        WidgetID result = a_ID; // self is the fallback
        node->ForEachChild([&](LayoutNode& child)
            {
                WidgetID childHit = HitTest(child.WidgetID, a_LogicalPos);
                if (childHit != c_InvalidPoolID)
                    result = childHit; // deepest child takes priority
            });

        return result;
    }

    bool UiScene::ProcessPointerEvent(const PointerEvent& a_Event)
    {
        if (!a_Event.IsMouse())
        {
            return false; // TODO: Only process mouse events for now
        }

        m_LastPointerEvent = a_Event;

        WidgetID hovered = HitTest(RootWidget, a_Event.Position);
        if (hovered != m_HoveredWidget)
        {
            if (IWidget* prevHovered = GetWidget(m_HoveredWidget))
                prevHovered->OnPointerExit(*this, a_Event);

            m_HoveredWidget = hovered;

            if (IWidget* newHovered = GetWidget(m_HoveredWidget))
                newHovered->OnPointerEnter(*this, a_Event);
        }

        return false;
    }

    bool UiScene::ProcessButtonEvent(const ButtonEvent& a_Event)
    {
        if (IWidget* hovered = GetWidget(m_HoveredWidget))
        {
            bool consumed = false;

            if (a_Event.Pressed)
                consumed |= hovered->OnPressed(*this, a_Event);
            else
                consumed |= hovered->OnReleased(*this, a_Event);

            if (consumed)
                return true; // Event handled by hovered widget
        }

        if (IWidget* focused = GetWidget(GetFocusedWidget()))
        {
            bool consumed = false;

            if (a_Event.Pressed)
                consumed |= focused->OnPressed(*this, a_Event);
            else
                consumed |= focused->OnReleased(*this, a_Event);

            if (consumed)
                return true; // Event handled by focused widget
        }

        return false; // Event not handled
    }

}