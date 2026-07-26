#include <catch2/catch_test_macros.hpp>
#include <helsinki/System/Utils/Json.hpp>

namespace hl
{

    namespace Test
    {

        TEST_CASE("Sample json token detection works", "[Utility][Json]")
        {
            const std::string text = R"json(
{
    "menu": {
        "id": "file",
        "value" : "File",
        "popup" : {
            "menuitem": [
                { "value": "New", "onclick" : "CreateNewDoc()"},
                { "value": "Open", "onclick" : "OpenDoc()" },
                { "value": "Close", "onclick" : "CloseDoc()" },
                { "value": 55.5, "onclick" : "CloseDoc()" },
                { "value": 555, "onclick" : "CloseDoc()" },
                { "value": false, "onclick" : "CloseDoc()" },
                { "value": true, "onclick" : "CloseDoc()" }
            ]
        }
    }
})json";

            JsonDocument doc;

            std::vector<JsonToken> tokens;
            doc.tokenFound = [&](JsonToken _token) -> void
                {
                    tokens.push_back(_token);
                };

            doc.parse(text);

            REQUIRE(91 == tokens.size());
        }

        TEST_CASE("Nested arrays work", "[Utility][Json]")
        {
            const std::string text = R"json([1, [2, [3], 2], 1])json";
            JsonDocument doc = Json::parseFromText(text);
            JsonNode& root = *doc.m_Root;

            {
                // root
                REQUIRE(root.name.empty());
                REQUIRE(JsonNode::Type::Array == root.type);
                REQUIRE(root.content.empty());
                REQUIRE(3 == root.children.size());

                {
                    // root[0]
                    JsonNode& current_0 = root[0];
                    REQUIRE(current_0.name.empty());
                    REQUIRE(JsonNode::Type::ValueInteger == current_0.type);
                    REQUIRE("1" == current_0.content);
                    REQUIRE(1 == current_0.integer);
                    REQUIRE(current_0.children.empty());
                }
                {
                    // root[1]
                    JsonNode& current_1 = root[1];
                    REQUIRE(current_1.name.empty());
                    REQUIRE(JsonNode::Type::Array == current_1.type);
                    REQUIRE(current_1.content.empty());
                    REQUIRE(3 == current_1.children.size());

                    {
                        // root[1][0]
                        JsonNode& current_1_0 = root[1][0];
                        REQUIRE(current_1_0.name.empty());
                        REQUIRE(JsonNode::Type::ValueInteger == current_1_0.type);
                        REQUIRE("2" == current_1_0.content);
                        REQUIRE(2 == current_1_0.integer);
                        REQUIRE(current_1_0.children.empty());
                    }
                    {
                        // root[1][1]
                        JsonNode& current_1_1 = root[1][1];
                        REQUIRE(current_1_1.name.empty());
                        REQUIRE(JsonNode::Type::Array == current_1_1.type);
                        REQUIRE(current_1_1.content.empty());
                        REQUIRE(1 == current_1_1.children.size());

                        {
                            // root[1][1][0]
                            JsonNode& current_1_1_0 = root[1][1][0];
                            REQUIRE(current_1_1_0.name.empty());
                            REQUIRE(JsonNode::Type::ValueInteger == current_1_1_0.type);
                            REQUIRE("3" == current_1_1_0.content);
                            REQUIRE(3 == current_1_1_0.integer);
                            REQUIRE(current_1_1_0.children.empty());
                        }
                    }
                    {
                        // root[1][2]
                        JsonNode& current_1_2 = root[1][2];
                        REQUIRE(current_1_2.name.empty());
                        REQUIRE(JsonNode::Type::ValueInteger == current_1_2.type);
                        REQUIRE("2" == current_1_2.content);
                        REQUIRE(2 == current_1_2.integer);
                        REQUIRE(current_1_2.children.empty());
                    }
                }
                {
                    // root[2]
                    JsonNode& current_2 = root[2];
                    REQUIRE(current_2.name.empty());
                    REQUIRE(JsonNode::Type::ValueInteger == current_2.type);
                    REQUIRE("1" == current_2.content);
                    REQUIRE(1 == current_2.integer);
                    REQUIRE(current_2.children.empty());
                }
            }
        }

        TEST_CASE("Advent of code json examples work", "[Utility][Json]")
        {
            SECTION("Array inside an object")
            {
                const std::string text = R"json({"d":"red","e":[1,2,3.2,"4x4"],"f":5})json";

                JsonDocument doc = Json::parseFromText(text);
                JsonNode& root = *doc.m_Root;

                {
                    // Root Node
                    REQUIRE(root.name.empty());
                    REQUIRE(JsonNode::Type::Object == root.type);
                    REQUIRE(root.content.empty());
                    REQUIRE(3 == root.children.size());

                    {
                        // d
                        JsonNode& current_d = root["d"];
                        REQUIRE("d" == current_d.name);
                        REQUIRE(JsonNode::Type::ValueString == current_d.type);
                        REQUIRE("red" == current_d.content);
                        REQUIRE(current_d.children.empty());
                    }
                    {
                        // e
                        JsonNode& current_e = root["e"];
                        REQUIRE("e" == current_e.name);
                        REQUIRE(JsonNode::Type::Array == current_e.type);
                        REQUIRE(current_e.content.empty());
                        REQUIRE(4 == current_e.children.size());

                        {
                            // e[0]
                            JsonNode& current_e_0 = root["e"][0];
                            REQUIRE(current_e_0.name.empty());
                            REQUIRE(JsonNode::Type::ValueInteger == current_e_0.type);
                            REQUIRE("1" == current_e_0.content);
                            REQUIRE(1 == current_e_0.integer);
                            REQUIRE(current_e_0.children.empty());
                        }
                        {
                            // e[1]
                            JsonNode& current_e_1 = root["e"][1];
                            REQUIRE(current_e_1.name.empty());
                            REQUIRE(JsonNode::Type::ValueInteger == current_e_1.type);
                            REQUIRE("2" == current_e_1.content);
                            REQUIRE(2 == current_e_1.integer);
                            REQUIRE(current_e_1.children.empty());
                        }
                        {
                            // e[2]
                            JsonNode& current_e_2 = root["e"][2];
                            REQUIRE(current_e_2.name.empty());
                            REQUIRE(JsonNode::Type::ValueNumber == current_e_2.type);
                            REQUIRE("3.2" == current_e_2.content);
                            REQUIRE(3.2f == current_e_2.number);
                            REQUIRE(current_e_2.children.empty());
                        }
                        {
                            // e[3]
                            JsonNode& current_e_3 = root["e"][3];
                            REQUIRE(current_e_3.name.empty());
                            REQUIRE(JsonNode::Type::ValueString == current_e_3.type);
                            REQUIRE("4x4" == current_e_3.content);
                            REQUIRE(current_e_3.children.empty());
                        }
                    }
                    {
                        // f
                        JsonNode& current_f = root["f"];
                        REQUIRE("f" == current_f.name);
                        REQUIRE(JsonNode::Type::ValueInteger == current_f.type);
                        REQUIRE("5" == current_f.content);
                        REQUIRE(5 == current_f.integer);
                        REQUIRE(current_f.children.empty());
                    }
                }
            }
        }

        TEST_CASE("Default streaming method creates json tree as expected", "[Utility][Json]")
        {
            const std::string text = R"json(
{
    "menu": {
        "id": "file",
        "value" : "File",
        "popup" : {
            "menuitem": [
                { "value": "New", "onclick" : "CreateNewDoc()"},
                { "value": 55.5, "onclick" : "CloseDoc()" },
                { "value": 555, "onclick" : "CloseDoc()" },
                { "value": true, "onclick" : "CloseDoc()" }
            ]
        }
    }
})json";
            JsonDocument doc = Json::parseFromText(text);
            JsonNode& root = *doc.m_Root;

            {
                // Root Node
                REQUIRE(root.name.empty());
                REQUIRE(JsonNode::Type::Object == root.type);
                REQUIRE(root.content.empty());
                REQUIRE(1 == root.children.size());

                {
                    // menu
                    JsonNode& menu = root["menu"];
                    REQUIRE("menu" == menu.name);
                    REQUIRE(JsonNode::Type::Object == menu.type);
                    REQUIRE(menu.content.empty());
                    REQUIRE(3 == menu.children.size());

                    {
                        {
                            // menu/id
                            JsonNode& menu_id = root["menu"]["id"];
                            REQUIRE("id" == menu_id.name);
                            REQUIRE(JsonNode::Type::ValueString == menu_id.type);
                            REQUIRE("file" == menu_id.content);
                            REQUIRE(menu_id.children.empty());
                        }
                        {
                            // menu/value
                            JsonNode& menu_value = root["menu"]["value"];
                            REQUIRE("value" == menu_value.name);
                            REQUIRE(JsonNode::Type::ValueString == menu_value.type);
                            REQUIRE("File" == menu_value.content);
                            REQUIRE(menu_value.children.empty());
                        }
                        {
                            // menu/popup
                            JsonNode& menu_popup = root["menu"]["popup"];
                            REQUIRE("popup" == menu_popup.name);
                            REQUIRE(JsonNode::Type::Object == menu_popup.type);
                            REQUIRE(menu_popup.content.empty());
                            REQUIRE(1 == menu_popup.children.size());

                            {
                                // menu/popup/menuitem
                                JsonNode& menu_popup_menuitem = root["menu"]["popup"]["menuitem"];
                                REQUIRE("menuitem" == menu_popup_menuitem.name);
                                REQUIRE(JsonNode::Type::Array == menu_popup_menuitem.type);
                                REQUIRE(menu_popup_menuitem.content.empty());
                                REQUIRE(4 == menu_popup_menuitem.children.size());

                                {
                                    // menu/popup/menuitem[0]
                                    JsonNode& menu_popup_menuitem_0 = root["menu"]["popup"]["menuitem"][0];
                                    REQUIRE(menu_popup_menuitem_0.name.empty());
                                    REQUIRE(JsonNode::Type::Object == menu_popup_menuitem_0.type);
                                    REQUIRE(menu_popup_menuitem_0.content.empty());
                                    REQUIRE(2 == menu_popup_menuitem_0.children.size());

                                    {
                                        // menu/popup/menuitem[0]/value
                                        JsonNode& menu_popup_menuitem_0_value = root["menu"]["popup"]["menuitem"][0]["value"];
                                        REQUIRE("value" == menu_popup_menuitem_0_value.name);
                                        REQUIRE(JsonNode::Type::ValueString == menu_popup_menuitem_0_value.type);
                                        REQUIRE("New" == menu_popup_menuitem_0_value.content);
                                        REQUIRE(menu_popup_menuitem_0_value.children.empty());
                                    }
                                    {
                                        // menu/popup/menuitem[0]/onclick
                                        JsonNode& menu_popup_menuitem_0_onclick = root["menu"]["popup"]["menuitem"][0]["onclick"];
                                        REQUIRE("onclick" == menu_popup_menuitem_0_onclick.name);
                                        REQUIRE(JsonNode::Type::ValueString == menu_popup_menuitem_0_onclick.type);
                                        REQUIRE("CreateNewDoc()" == menu_popup_menuitem_0_onclick.content);
                                        REQUIRE(menu_popup_menuitem_0_onclick.children.empty());
                                    }
                                }
                                {
                                    // menu/popup/menuitem[1]
                                    JsonNode& menu_popup_menuitem_1 = root["menu"]["popup"]["menuitem"][1];
                                    REQUIRE(menu_popup_menuitem_1.name.empty());
                                    REQUIRE(JsonNode::Type::Object == menu_popup_menuitem_1.type);
                                    REQUIRE(menu_popup_menuitem_1.content.empty());
                                    REQUIRE(2 == menu_popup_menuitem_1.children.size());

                                    {
                                        // menu/popup/menuitem[1]/value
                                        JsonNode& menu_popup_menuitem_1_value = root["menu"]["popup"]["menuitem"][1]["value"];
                                        REQUIRE("value" == menu_popup_menuitem_1_value.name);
                                        REQUIRE(JsonNode::Type::ValueNumber == menu_popup_menuitem_1_value.type);
                                        REQUIRE("55.5" == menu_popup_menuitem_1_value.content);
                                        REQUIRE(55.5 == menu_popup_menuitem_1_value.number);
                                        REQUIRE(menu_popup_menuitem_1_value.children.empty());
                                    }
                                    {
                                        // menu/popup/menuitem[1]/onclick
                                        JsonNode& menu_popup_menuitem_1_onclick = root["menu"]["popup"]["menuitem"][1]["onclick"];
                                        REQUIRE("onclick" == menu_popup_menuitem_1_onclick.name);
                                        REQUIRE(JsonNode::Type::ValueString == menu_popup_menuitem_1_onclick.type);
                                        REQUIRE("CloseDoc()" == menu_popup_menuitem_1_onclick.content);
                                        REQUIRE(menu_popup_menuitem_1_onclick.children.empty());
                                    }
                                }
                                {
                                    // menu/popup/menuitem[2]
                                    JsonNode& menu_popup_menuitem_2 = root["menu"]["popup"]["menuitem"][2];
                                    REQUIRE(menu_popup_menuitem_2.name.empty());
                                    REQUIRE(JsonNode::Type::Object == menu_popup_menuitem_2.type);
                                    REQUIRE(menu_popup_menuitem_2.content.empty());
                                    REQUIRE(2 == menu_popup_menuitem_2.children.size());

                                    {
                                        // menu/popup/menuitem[2]/value
                                        JsonNode& menu_popup_menuitem_2_value = root["menu"]["popup"]["menuitem"][2]["value"];
                                        REQUIRE("value" == menu_popup_menuitem_2_value.name);
                                        REQUIRE(JsonNode::Type::ValueInteger == menu_popup_menuitem_2_value.type);
                                        REQUIRE("555" == menu_popup_menuitem_2_value.content);
                                        REQUIRE(555 == menu_popup_menuitem_2_value.integer);
                                        REQUIRE(menu_popup_menuitem_2_value.children.empty());
                                    }
                                    {
                                        // menu/popup/menuitem[2]/onclick
                                        JsonNode& menu_popup_menuitem_2_onclick = root["menu"]["popup"]["menuitem"][2]["onclick"];
                                        REQUIRE("onclick" == menu_popup_menuitem_2_onclick.name);
                                        REQUIRE(JsonNode::Type::ValueString == menu_popup_menuitem_2_onclick.type);
                                        REQUIRE("CloseDoc()" == menu_popup_menuitem_2_onclick.content);
                                        REQUIRE(menu_popup_menuitem_2_onclick.children.empty());
                                    }
                                }
                                {
                                    // menu/popup/menuitem[3]
                                    JsonNode& menu_popup_menuitem_3 = root["menu"]["popup"]["menuitem"][3];
                                    REQUIRE(menu_popup_menuitem_3.name.empty());
                                    REQUIRE(JsonNode::Type::Object == menu_popup_menuitem_3.type);
                                    REQUIRE(menu_popup_menuitem_3.content.empty());
                                    REQUIRE(2 == menu_popup_menuitem_3.children.size());

                                    {
                                        // menu/popup/menuitem[3]/value
                                        JsonNode& menu_popup_menuitem_3_value = root["menu"]["popup"]["menuitem"][3]["value"];
                                        REQUIRE("value" == menu_popup_menuitem_3_value.name);
                                        REQUIRE(JsonNode::Type::ValueBoolean == menu_popup_menuitem_3_value.type);
                                        REQUIRE("true" == menu_popup_menuitem_3_value.content);
                                        REQUIRE(menu_popup_menuitem_3_value.boolean);
                                        REQUIRE(menu_popup_menuitem_3_value.children.empty());
                                    }
                                    {
                                        // menu/popup/menuitem[3]/onclick
                                        JsonNode& menu_popup_menuitem_3_onclick = root["menu"]["popup"]["menuitem"][3]["onclick"];
                                        REQUIRE("onclick" == menu_popup_menuitem_3_onclick.name);
                                        REQUIRE(JsonNode::Type::ValueString == menu_popup_menuitem_3_onclick.type);
                                        REQUIRE("CloseDoc()" == menu_popup_menuitem_3_onclick.content);
                                        REQUIRE(menu_popup_menuitem_3_onclick.children.empty());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    }
}