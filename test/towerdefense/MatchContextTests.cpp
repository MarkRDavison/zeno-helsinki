#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
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

TEST_CASE("skirmish start gold is the level value", "[tower][match]")
{
	tower::MatchContext match;
	CHECK(tower::matchStartGold(100, match) == 100);
}

TEST_CASE("campaign start gold adds rank bonus", "[tower][match]")
{
	tower::MatchContext match;
	match.campaign = true;
	CHECK(tower::matchStartGold(100, match) == 100);
	match.startingGoldRank = 1;
	CHECK(tower::matchStartGold(100, match) == 110);
	match.startingGoldRank = 2;
	CHECK(tower::matchStartGold(100, match) == 120);
}

TEST_CASE("skirmish fire cooldown is the catalog value", "[tower][match]")
{
	tower::MatchContext match;
	CHECK(tower::matchFireCooldown(0.7f, match) == 0.7f);
}

TEST_CASE("campaign fire cooldown scales per rank", "[tower][match]")
{
	tower::MatchContext match;
	match.campaign = true;
	CHECK(tower::matchFireCooldown(0.7f, match) == Catch::Approx(0.7f));
	match.fireRateRank = 1;
	CHECK(
		tower::matchFireCooldown(0.7f, match)
		== Catch::Approx(0.7f * tower::kFireRateCooldownScale));
	match.fireRateRank = 2;
	CHECK(
		tower::matchFireCooldown(0.7f, match)
		== Catch::Approx(0.7f * tower::kFireRateCooldownScale * tower::kFireRateCooldownScale));
}
