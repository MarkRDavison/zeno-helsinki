#include <catch2/catch_test_macros.hpp>
#include <Services/MatchContext.hpp>

TEST_CASE("campaign match lists owned towers only", "[tower][match]")
{
	const std::vector<std::string> catalog{ "single", "double" };
	tower::MatchContext match;
	match.campaign = true;
	match.ownedTowers = { "single" };
	CHECK(tower::matchPlaceableTowerIds(catalog, match) == std::vector<std::string>{ "single" });
}

TEST_CASE("campaign match lists owned towers in catalog order", "[tower][match]")
{
	const std::vector<std::string> catalog{ "single", "double" };
	tower::MatchContext match;
	match.campaign = true;
	match.ownedTowers = { "double", "single" };
	CHECK(
		tower::matchPlaceableTowerIds(catalog, match)
		== std::vector<std::string>{ "single", "double" });
}

TEST_CASE("skirmish match lists full catalog", "[tower][match]")
{
	const std::vector<std::string> catalog{ "single", "double" };
	tower::MatchContext match;
	CHECK(tower::matchPlaceableTowerIds(catalog, match) == catalog);
}

TEST_CASE("campaign match ignores unknown owned ids", "[tower][match]")
{
	const std::vector<std::string> catalog{ "single", "double" };
	tower::MatchContext match;
	match.campaign = true;
	match.ownedTowers = { "single", "laser" };
	CHECK(tower::matchPlaceableTowerIds(catalog, match) == std::vector<std::string>{ "single" });
}
