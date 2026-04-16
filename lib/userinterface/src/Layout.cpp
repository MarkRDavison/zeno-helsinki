#include <helsinki/UserInterface/Layout.hpp>

namespace hl
{
    void LayoutNode::DetachFromParent()
    {
        if (!Parent)
        {
            return; // Not attached to any parent
        }

        // Link siblings together, bypassing this node
        if (PrevSibling)
        {
            PrevSibling->NextSibling = NextSibling;
        }
        else
        {
            Parent->FirstChild = NextSibling;
        }

        if (NextSibling)
        {
            NextSibling->PrevSibling = PrevSibling;
        }
        else
        {
            Parent->LastChild = PrevSibling;
        }

        Parent->NumChildren--;
        Parent = nullptr;
        PrevSibling = nullptr;
        NextSibling = nullptr;
    }

    void LayoutNode::PushBackChild(LayoutNode& a_Child)
    {
        a_Child.DetachFromParent(); // Ensure child is not currently attached to another parent

        a_Child.Parent = this;
        a_Child.NextSibling = nullptr;

        if (LastChild)
        {
            LastChild->NextSibling = &a_Child;
            a_Child.PrevSibling = LastChild;
        }
        else
        {
            FirstChild = &a_Child;
            a_Child.PrevSibling = nullptr;
        }

        LastChild = &a_Child;
        ++NumChildren;
    }

    void LayoutNode::PushFrontChild(LayoutNode& a_Child)
    {
        a_Child.DetachFromParent(); // Ensure child is not currently attached to another parent

        a_Child.Parent = this;
        a_Child.PrevSibling = nullptr;

        if (FirstChild)
        {
            FirstChild->PrevSibling = &a_Child;
            a_Child.NextSibling = FirstChild;
        }
        else
        {
            LastChild = &a_Child;
            a_Child.NextSibling = nullptr;
        }

        FirstChild = &a_Child;
        ++NumChildren;
    }

    void LayoutNode::InsertChildAfter(LayoutNode& a_Child, LayoutNode& a_Sibling)
    {
        if (!a_Sibling.Parent || a_Sibling.Parent != this)
        {
            assert(false && "Sibling node is not a child of this parent");
            return;
        }

        a_Child.DetachFromParent(); // Ensure child is not currently attached to another parent

        a_Child.Parent = this;
        a_Child.PrevSibling = &a_Sibling;
        a_Child.NextSibling = a_Sibling.NextSibling;

        if (a_Sibling.NextSibling)
            a_Sibling.NextSibling->PrevSibling = &a_Child;
        else
            LastChild = &a_Child;

        a_Sibling.NextSibling = &a_Child;
        ++NumChildren;
    }

    void LayoutNode::InsertChildBefore(LayoutNode& a_Child, LayoutNode& a_Sibling)
    {
        if (!a_Sibling.Parent || a_Sibling.Parent != this)
        {
            assert(false && "Sibling node is not a child of this parent");
            return;
        }

        a_Child.DetachFromParent(); // Ensure child is not currently attached to another parent

        a_Child.Parent = this;
        a_Child.NextSibling = &a_Sibling;
        a_Child.PrevSibling = a_Sibling.PrevSibling;

        if (a_Sibling.PrevSibling)
        {
            a_Sibling.PrevSibling->NextSibling = &a_Child;
        }
        else
        {
            FirstChild = &a_Child;
        }

        a_Sibling.PrevSibling = &a_Child;
        ++NumChildren;
    }
}