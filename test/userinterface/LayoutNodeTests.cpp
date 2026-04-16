#include <catch2/catch_test_macros.hpp>
#include <fakeit.hpp>
#include <helsinki/UserInterface/LayoutEngine.hpp>
#include "../TestHelpers.hpp"

namespace hl
{
	namespace tests
	{

        TEST_CASE("UserInterface::LayoutNode::PushBackChild sets parent pointer and NumChildren", "[layoutnode][hierarchy]")
        {
            LayoutNode parent{};
            LayoutNode child{};

            parent.PushBackChild(child);

            REQUIRE(parent.NumChildren == 1);
            REQUIRE(parent.FirstChild == &child);
            REQUIRE(parent.LastChild == &child);
            REQUIRE(child.Parent == &parent);
            REQUIRE(child.PrevSibling == nullptr);
            REQUIRE(child.NextSibling == nullptr);
        }

        TEST_CASE("UserInterface::LayoutNode::PushBackChild with two children maintains correct sibling links", "[layoutnode][hierarchy]")
        {
            LayoutNode parent{};
            LayoutNode first{};
            LayoutNode second{};

            parent.PushBackChild(first);
            parent.PushBackChild(second);

            REQUIRE(parent.NumChildren == 2);
            REQUIRE(parent.FirstChild == &first);
            REQUIRE(parent.LastChild == &second);
            REQUIRE(first.NextSibling == &second);
            REQUIRE(second.PrevSibling == &first);
            REQUIRE(first.PrevSibling == nullptr);
            REQUIRE(second.NextSibling == nullptr);
        }

        TEST_CASE("UserInterface::LayoutNode::PushBackChild with three children preserves order", "[layoutnode][hierarchy]")
        {
            LayoutNode parent{};
            LayoutNode a{}, b{}, c{};

            parent.PushBackChild(a);
            parent.PushBackChild(b);
            parent.PushBackChild(c);

            REQUIRE(parent.NumChildren == 3);
            REQUIRE(parent.FirstChild == &a);
            REQUIRE(parent.LastChild == &c);
            REQUIRE(a.NextSibling == &b);
            REQUIRE(b.NextSibling == &c);
            REQUIRE(c.PrevSibling == &b);
            REQUIRE(b.PrevSibling == &a);
        }

        TEST_CASE("UserInterface::LayoutNode::PushBackChild detaches child from previous parent", "[layoutnode][hierarchy]")
        {
            LayoutNode parent1{};
            LayoutNode parent2{};
            LayoutNode child{};

            parent1.PushBackChild(child);
            REQUIRE(parent1.NumChildren == 1);

            parent2.PushBackChild(child);
            REQUIRE(parent1.NumChildren == 0);
            REQUIRE(parent2.NumChildren == 1);
            REQUIRE(child.Parent == &parent2);
            REQUIRE(parent1.FirstChild == nullptr);
            REQUIRE(parent1.LastChild == nullptr);
        }

        TEST_CASE("UserInterface::LayoutNode::PushBackChild with two children puts second child at front", "[layoutnode][hierarchy]")
        {
            LayoutNode parent{};
            LayoutNode first{};
            LayoutNode second{};

            parent.PushFrontChild(first);
            parent.PushFrontChild(second);

            REQUIRE(parent.NumChildren == 2);
            REQUIRE(parent.FirstChild == &second);
            REQUIRE(parent.LastChild == &first);
            REQUIRE(second.NextSibling == &first);
            REQUIRE(first.PrevSibling == &second);
        }

        TEST_CASE("UserInterface::LayoutNode::PushBackChild with three children preserves reverse insertion order", "[layoutnode][hierarchy]")
        {
            LayoutNode parent{};
            LayoutNode a{}, b{}, c{};

            parent.PushFrontChild(a);
            parent.PushFrontChild(b);
            parent.PushFrontChild(c);

            // Front: c → b → a :Back
            REQUIRE(parent.FirstChild == &c);
            REQUIRE(parent.LastChild == &a);
            REQUIRE(c.NextSibling == &b);
            REQUIRE(b.NextSibling == &a);
            REQUIRE(a.PrevSibling == &b);
            REQUIRE(b.PrevSibling == &c);
        }

        TEST_CASE("UserInterface::LayoutNode::DetachFromParent on only child clears parent FirstChild and LastChild", "[layoutnode][hierarchy]")
        {
            LayoutNode parent{};
            LayoutNode child{};

            parent.PushBackChild(child);
            child.DetachFromParent();

            REQUIRE(parent.NumChildren == 0);
            REQUIRE(parent.FirstChild == nullptr);
            REQUIRE(parent.LastChild == nullptr);
            REQUIRE(child.Parent == nullptr);
            REQUIRE(child.PrevSibling == nullptr);
            REQUIRE(child.NextSibling == nullptr);
        }

        TEST_CASE("UserInterface::LayoutNode::DetachFromParent on first child re-links remaining children", "[layoutnode][hierarchy]")
        {
            LayoutNode parent{};
            LayoutNode a{}, b{}, c{};
            parent.PushBackChild(a);
            parent.PushBackChild(b);
            parent.PushBackChild(c);

            a.DetachFromParent();

            REQUIRE(parent.NumChildren == 2);
            REQUIRE(parent.FirstChild == &b);
            REQUIRE(b.PrevSibling == nullptr);
            REQUIRE(b.NextSibling == &c);
            REQUIRE(c.PrevSibling == &b);
        }

        TEST_CASE("UserInterface::LayoutNode::DetachFromParent on middle child re-links siblings", "[layoutnode][hierarchy]")
        {
            LayoutNode parent{};
            LayoutNode a{}, b{}, c{};
            parent.PushBackChild(a);
            parent.PushBackChild(b);
            parent.PushBackChild(c);

            b.DetachFromParent();

            REQUIRE(parent.NumChildren == 2);
            REQUIRE(a.NextSibling == &c);
            REQUIRE(c.PrevSibling == &a);
            REQUIRE(parent.FirstChild == &a);
            REQUIRE(parent.LastChild == &c);
        }

        TEST_CASE("UserInterface::LayoutNode::DetachFromParent on last child re-links remaining children", "[layoutnode][hierarchy]")
        {
            LayoutNode parent{};
            LayoutNode a{}, b{};
            parent.PushBackChild(a);
            parent.PushBackChild(b);

            b.DetachFromParent();

            REQUIRE(parent.NumChildren == 1);
            REQUIRE(parent.LastChild == &a);
            REQUIRE(a.NextSibling == nullptr);
        }

        TEST_CASE("UserInterface::LayoutNode::DetachFromParent on a node with no parent is a no-op", "[layoutnode][hierarchy]")
        {
            LayoutNode node{};
            node.DetachFromParent(); // should not crash
            REQUIRE(node.Parent == nullptr);
        }

        TEST_CASE("UserInterface::LayoutNode::InsertChildAfter places new child immediately after the given sibling", "[layoutnode][hierarchy]")
        {
            LayoutNode parent{};
            LayoutNode a{}, b{}, c{};

            parent.PushBackChild(a);
            parent.PushBackChild(c);
            parent.InsertChildAfter(b, a); // a → b → c

            REQUIRE(parent.NumChildren == 3);
            REQUIRE(parent.FirstChild == &a);
            REQUIRE(parent.LastChild == &c);
            REQUIRE(a.NextSibling == &b);
            REQUIRE(b.PrevSibling == &a);
            REQUIRE(b.NextSibling == &c);
            REQUIRE(c.PrevSibling == &b);
        }

        TEST_CASE("UserInterface::LayoutNode::InsertChildAfter at the last position updates LastChild", "[layoutnode][hierarchy]")
        {
            LayoutNode parent{};
            LayoutNode a{}, b{};

            parent.PushBackChild(a);
            parent.InsertChildAfter(b, a); // a → b

            REQUIRE(parent.NumChildren == 2);
            REQUIRE(parent.LastChild == &b);
            REQUIRE(a.NextSibling == &b);
            REQUIRE(b.PrevSibling == &a);
            REQUIRE(b.NextSibling == nullptr);
        }

        TEST_CASE("UserInterface::LayoutNode::InsertChildBefore places new child immediately before the given sibling", "[layoutnode][hierarchy]")
        {
            LayoutNode parent{};
            LayoutNode a{}, b{}, c{};

            parent.PushBackChild(a);
            parent.PushBackChild(c);
            parent.InsertChildBefore(b, c); // a → b → c

            REQUIRE(parent.NumChildren == 3);
            REQUIRE(parent.FirstChild == &a);
            REQUIRE(parent.LastChild == &c);
            REQUIRE(a.NextSibling == &b);
            REQUIRE(b.PrevSibling == &a);
            REQUIRE(b.NextSibling == &c);
            REQUIRE(c.PrevSibling == &b);
        }

        TEST_CASE("UserInterface::LayoutNode::InsertChildBefore at the first position updates FirstChild", "[layoutnode][hierarchy]")
        {
            LayoutNode parent{};
            LayoutNode a{}, b{};

            parent.PushBackChild(b);
            parent.InsertChildBefore(a, b); // a → b

            REQUIRE(parent.NumChildren == 2);
            REQUIRE(parent.FirstChild == &a);
            REQUIRE(a.PrevSibling == nullptr);
            REQUIRE(a.NextSibling == &b);
            REQUIRE(b.PrevSibling == &a);
        }

        TEST_CASE("UserInterface::LayoutNode::ForEachChild visits each child exactly once in insertion order", "[layoutnode][hierarchy]")
        {
            LayoutNode parent{};
            LayoutNode a{}, b{}, c{};
            parent.PushBackChild(a);
            parent.PushBackChild(b);
            parent.PushBackChild(c);

            std::vector<LayoutNode*> visited{};
            parent.ForEachChild([&](LayoutNode& node) { visited.push_back(&node); });

            REQUIRE(visited.size() == 3);
            REQUIRE(visited[0] == &a);
            REQUIRE(visited[1] == &b);
            REQUIRE(visited[2] == &c);
        }

        TEST_CASE("UserInterface::LayoutNode::ForEachChild on empty parent visits nothing", "[layoutnode][hierarchy]")
        {
            LayoutNode parent{};
            int count = 0;
            parent.ForEachChild([&](LayoutNode&) { ++count; });
            REQUIRE(count == 0);
        }

        TEST_CASE("UserInterface::LayoutNode::ForEachChild const version visits each child in order", "[layoutnode][hierarchy]")
        {
            LayoutNode parent{};
            LayoutNode a{}, b{};
            parent.PushBackChild(a);
            parent.PushBackChild(b);

            const LayoutNode& constParent = parent;
            std::vector<const LayoutNode*> visited{};
            constParent.ForEachChild([&](const LayoutNode& node) { visited.push_back(&node); });

            REQUIRE(visited.size() == 2);
            REQUIRE(visited[0] == &a);
            REQUIRE(visited[1] == &b);
        }

        TEST_CASE("UserInterface::LayoutNode::ForEachDescendant visits all nodes in depth-first order", "[layoutnode][hierarchy]")
        {
            // Tree structure:
            //   root
            //   ├── a
            //   │   └── a1
            //   └── b
            LayoutNode root{};
            LayoutNode a{}, a1{}, b{};

            root.PushBackChild(a);
            root.PushBackChild(b);
            a.PushBackChild(a1);

            std::vector<LayoutNode*> visited{};
            root.ForEachDescendant([&](LayoutNode& node) { visited.push_back(&node); });

            // depth-first: a, a1, b
            REQUIRE(visited.size() == 3);
            REQUIRE(visited[0] == &a);
            REQUIRE(visited[1] == &a1);
            REQUIRE(visited[2] == &b);
        }

        TEST_CASE("UserInterface::LayoutNode::ForEachDescendant on a leaf visits nothing", "[layoutnode][hierarchy]")
        {
            LayoutNode leaf{};
            int count = 0;
            leaf.ForEachDescendant([&](LayoutNode&) { ++count; });
            REQUIRE(count == 0);
        }

        TEST_CASE("UserInterface::LayoutNode::ForEachDescendant visits deeply nested nodes", "[layoutnode][hierarchy]")
        {
            // root → a → b → c (linear chain)
            LayoutNode root{}, a{}, b{}, c{};
            root.PushBackChild(a);
            a.PushBackChild(b);
            b.PushBackChild(c);

            std::vector<LayoutNode*> visited{};
            root.ForEachDescendant([&](LayoutNode& node) { visited.push_back(&node); });

            REQUIRE(visited.size() == 3);
            REQUIRE(visited[0] == &a);
            REQUIRE(visited[1] == &b);
            REQUIRE(visited[2] == &c);
        }

	}
}