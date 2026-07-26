#include <catch2/catch_test_macros.hpp>
#include <helsinki/System/Utils/Xml.hpp>

#define INVALID_INDEX 0xffffffff

namespace hl
{

    namespace Test
    {

        TEST_CASE("A valid piece of text can be read", "[Utility][Xml]")
        {
            const std::string text =
                "<root><child><grandchild></grandchild></child></root>";

            XmlDocument doc;
            REQUIRE(doc.readFromText(text));

            REQUIRE(text == doc.getInputRepresentation());
        }

        TEST_CASE("A simple non self closing doc validates", "[Utility][Xml]")
        {
            const std::string text =
                "<root><child><grandchild></grandchild></child></root>";

            REQUIRE(Xml::validate(text));
        }

        TEST_CASE("A simple invalid doc validates", "[Utility][Xml]")
        {
            const std::string text =
                "<root>>";

            REQUIRE_FALSE(Xml::validate(text));
        }

        TEST_CASE("getNextTagStartIndex where there is no start returns invalid", "[Utility][Xml]")
        {
            REQUIRE(INVALID_INDEX == XmlDocument::getNextTagStartIndex("<root>", 1));
        }

        TEST_CASE("getNextTagStartIndex where it starts with the start returns 0", "[Utility][Xml]")
        {
            REQUIRE(0 == XmlDocument::getNextTagStartIndex("<root>", 0));
        }

        TEST_CASE("getNextTagStartIndex where the start is later returns correctly", "[Utility][Xml]")
        {
            REQUIRE(6 == XmlDocument::getNextTagStartIndex("<root></root>", 1));
        }

        TEST_CASE("Parsing single open and close element with attributes streams events as expected", "[Utility][Xml]")
        {
            const std::string text = "<root attribute1='value1' attribute2=\"value2\"></root>";

            XmlDocument doc;
            REQUIRE(doc.readFromText(text));

            const unsigned expectedItems = 2;
            std::vector<ItemDiscovered> items;

            doc.itemDiscovered = [&](ItemDiscovered _item) -> void
                {
                    items.push_back(_item);
                };

            REQUIRE(doc.parse(doc.getInputRepresentation()));
            REQUIRE(expectedItems == items.size());

            ItemDiscovered rootOpen = items[0];
            ItemDiscovered rootClose = items[1];

            REQUIRE(Type::OpenElement == rootOpen.type);
            REQUIRE("root" == rootOpen.name);
            REQUIRE(2 == rootOpen.attributes.size());

            REQUIRE("value1" == rootOpen.attributes["attribute1"]);

            REQUIRE("value2" == rootOpen.attributes["attribute2"]);

            REQUIRE(Type::CloseElement == rootClose.type);
            REQUIRE("root" == rootClose.name);
        }

        TEST_CASE("Single layer of nesting works", "[Utility][Xml]")
        {
            const std::string text = "<root attribute1='value1' attribute2=\"value2\"><child c_attribute1='value1' c_attribute2=\"value2\"><grandchild g_attribute1='value1' g_attribute2=\"value2\"></grandchild></child></root>";

            XmlDocument doc;
            REQUIRE(doc.readFromText(text));

            std::vector<ItemDiscovered> items;

            doc.itemDiscovered = [&](ItemDiscovered _item) -> void
                {
                    items.push_back(_item);
                };

            REQUIRE(doc.parse(doc.getInputRepresentation()));

            REQUIRE(6 == items.size());
            REQUIRE(Type::OpenElement == items[0].type);
            REQUIRE(2 == items[0].attributes.size());
            REQUIRE(Type::OpenElement == items[1].type);
            REQUIRE(2 == items[1].attributes.size());
            REQUIRE(Type::OpenElement == items[2].type);
            REQUIRE(2 == items[2].attributes.size());
            REQUIRE(Type::CloseElement == items[3].type);
            REQUIRE(Type::CloseElement == items[4].type);
            REQUIRE(Type::CloseElement == items[5].type);
        }

        TEST_CASE("Single layer of nesting works when document has whitespace", "[Utility][Xml]")
        {
            const std::string text = R"xml(
<root attribute1='value1' attribute2="value2">
    <child c_attribute1='value1' c_attribute2="value2">
        <grandchild g_attribute1='value1' g_attribute2="value2">
        </grandchild>
    </child>
</root>
)xml";

            XmlDocument doc;
            REQUIRE(doc.readFromText(text));

            std::vector<ItemDiscovered> items;

            doc.itemDiscovered = [&](ItemDiscovered _item) -> void
                {
                    items.push_back(_item);
                };

            REQUIRE(doc.parse(doc.getInputRepresentation()));

            REQUIRE(6 == items.size());
            REQUIRE(Type::OpenElement == items[0].type);
            REQUIRE(2 == items[0].attributes.size());
            REQUIRE(Type::OpenElement == items[1].type);
            REQUIRE(2 == items[1].attributes.size());
            REQUIRE(Type::OpenElement == items[2].type);
            REQUIRE(2 == items[2].attributes.size());
            REQUIRE(Type::CloseElement == items[3].type);
            REQUIRE(Type::CloseElement == items[4].type);
            REQUIRE(Type::CloseElement == items[5].type);
        }

        TEST_CASE("Single layer of nesting works when document has whitespace and grandchild is self closing", "[Utility][Xml]")
        {
            const std::string text = R"xml(
<root attribute1='value1' attribute2="value2">
    <child c_attribute1='value1' c_attribute2="value2">
        <grandchild g_attribute1='value1' g_attribute2="value2"/>
    </child>
</root>
)xml";

            XmlDocument doc;
            REQUIRE(doc.readFromText(text));

            std::vector<ItemDiscovered> items;

            doc.itemDiscovered = [&](ItemDiscovered _item) -> void
                {
                    items.push_back(_item);
                };

            REQUIRE(doc.parse(doc.getInputRepresentation()));

            REQUIRE(5 == items.size());
            REQUIRE(Type::OpenElement == items[0].type);
            REQUIRE(2 == items[0].attributes.size());
            REQUIRE(Type::OpenElement == items[1].type);
            REQUIRE(2 == items[1].attributes.size());
            REQUIRE(Type::SelfCloseElement == items[2].type);
            REQUIRE(2 == items[2].attributes.size());
            REQUIRE(Type::CloseElement == items[3].type);
            REQUIRE(Type::CloseElement == items[4].type);
        }

        TEST_CASE("xml with content is streamed correctly", "[Utility][Xml]")
        {
            const std::string text = R"xml(
<root attribute1='value1' attribute2="value2">
    <child c_attribute1='value1' c_attribute2="value2">
        <grandchild>I'm some content</grandchild>
    </child>
</root>
)xml";

            XmlDocument doc;
            REQUIRE(doc.readFromText(text));

            std::vector<ItemDiscovered> items;

            doc.itemDiscovered = [&](ItemDiscovered _item) -> void
                {
                    items.push_back(_item);
                };

            REQUIRE(doc.parse(doc.getInputRepresentation()));

            REQUIRE(7 == items.size());
            REQUIRE(Type::OpenElement == items[0].type);
            REQUIRE(2 == items[0].attributes.size());
            REQUIRE(Type::OpenElement == items[1].type);
            REQUIRE(2 == items[1].attributes.size());
            REQUIRE(Type::OpenElement == items[2].type);
            REQUIRE(0 == items[2].attributes.size());
            REQUIRE(Type::Content == items[3].type);
            REQUIRE("I'm some content" == items[3].content);
            REQUIRE(Type::CloseElement == items[4].type);
            REQUIRE(Type::CloseElement == items[5].type);
            REQUIRE(Type::CloseElement == items[6].type);
        }

        TEST_CASE("Can use default methods to stream document to tree", "[Utility][Xml]")
        {
            const std::string text = R"xml(
<root attribute1='value1' attribute2="value2">
    <child c_attribute1='value1' c_attribute2="value2">
        <grandchild1>I'm some content</grandchild1>
        <grandchild2 gc_attribute1='value1'/>
    </child>
</root>
)xml";

            XmlDocument doc = Xml::parseFromText(text);

            auto current = doc.m_Node;

            REQUIRE("root" == current->name);
            current = current->children[0];
            REQUIRE("grandchild1" == current->children[0]->name);
            REQUIRE("grandchild2" == current->children[1]->name);
        }

        TEST_CASE("Select on basic single width tree at various depths works", "[Utility][Xml]")
        {
            const std::string text = R"xml(
<root>
    <child>
        <grandchild>
            <greatgrandchild/>
        </grandchild>
    </child>
</root>
)xml";
            XmlDocument doc = Xml::parseFromText(text);

            REQUIRE("root" == doc.select("root")->name);
            REQUIRE("child" == doc.select("root/child")->name);
            REQUIRE("grandchild" == doc.select("root/child/grandchild")->name);
            REQUIRE("greatgrandchild" == doc.select("root/child/grandchild/greatgrandchild")->name);

        }

        TEST_CASE("SelectMany on basic multi width tree at various depths works", "[Utility][Xml]")
        {
            const std::string text = R"xml(
<root>
    <child>
        <grandchild>
            <greatgrandchild/>
            <greatgrandchild/>
            <greatgrandchild/>
            <greatgrandchild/>
        </grandchild>
        <grandchild>
            <greatgrandchild/>
            <greatgrandchild/>
            <greatgrandchild/>
            <greatgrandchild/>
        </grandchild>
        <grandchild>
            <greatgrandchild/>
            <greatgrandchild/>
            <greatgrandchild/>
            <greatgrandchild/>
        </grandchild>
    </child>
    <child>
        <grandchild>
            <greatgrandchild/>
            <greatgrandchild/>
            <greatgrandchild/>
            <greatgrandchild/>
        </grandchild>
        <grandchild>
            <greatgrandchild/>
            <greatgrandchild/>
            <greatgrandchild/>
            <greatgrandchild/>
        </grandchild>
        <grandchild>
            <greatgrandchild/>
            <greatgrandchild/>
            <greatgrandchild/>
            <greatgrandchild/>
        </grandchild>
    </child>
</root>
)xml";
            XmlDocument doc = Xml::parseFromText(text);

            const auto& root = doc.selectMany("root");
            REQUIRE(1 == root.size());
            const auto& child = doc.selectMany("root/child");
            REQUIRE(2 == child.size());
            const auto& grandchild = doc.selectMany("root/child/grandchild");
            REQUIRE(3 == grandchild.size());
            const auto& greatgrandchild = doc.selectMany("root/child/grandchild/greatgrandchild");
            REQUIRE(4 == greatgrandchild.size());

        }
    }


    TEST_CASE("xml getAttribute methods work", "[Utility][Xml]")
    {
        const std::string text = R"xml(
<root a1='value1' a2="true" a3="123" a4="222" a5="1.5">
</root>
)xml";

        XmlDocument doc = Xml::parseFromText(text);

        REQUIRE("value1" == hl::Xml::getAttribute<std::string>(doc.m_Node, "a1"));
        REQUIRE(true == hl::Xml::getAttribute<bool>(doc.m_Node, "a2"));
        REQUIRE(123u == hl::Xml::getAttribute<unsigned>(doc.m_Node, "a3"));
        REQUIRE(222 == hl::Xml::getAttribute<int>(doc.m_Node, "a4"));
        REQUIRE(1.5f == hl::Xml::getAttribute<float>(doc.m_Node, "a5"));

    }
}