#include <catch2/catch_test_macros.hpp>
#include <helsinki/Renderer/Resource/TextSystem.hpp>
#include <vector>

TEST_CASE("fontAtlasIndex returns child order", "[text]")
{
	const std::vector<std::string> children{ "consolab", "other" };
	REQUIRE(hl::fontAtlasIndex(children, "consolab") == 0);
	REQUIRE(hl::fontAtlasIndex(children, "other") == 1);
}

TEST_CASE("fontAtlasIndex throws when font is missing", "[text]")
{
	const std::vector<std::string> children{ "roboto" };
	REQUIRE_THROWS_AS(hl::fontAtlasIndex(children, "missing"), std::runtime_error);
}
