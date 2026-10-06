#include <catch2/catch_test_macros.hpp>
#include <Services/GraphEvaluator.hpp>
#include <Services/ProfileService.hpp>
#include <Services/ResearchCatalog.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace
{
	std::filesystem::path uniqueSavePath()
	{
		const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
		auto dir = std::filesystem::temp_directory_path()
			/ ("helsinki-td-research-" + std::to_string(stamp));
		std::filesystem::create_directories(dir);
		return dir / "save.json";
	}

	void writeText(const std::filesystem::path& path, const std::string& text)
	{
		std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
		file << text;
	}

	constexpr auto kResearchJson = R"json(
{
  "version": 1,
  "nodes": [
    { "id": "single", "label": "Single", "start": true, "cost": 0, "effect": { "tower": "single" } },
    { "id": "double", "label": "Double", "cost": 100, "prereq": { "all": ["single"] }, "effect": { "tower": "double" } },
    { "id": "gold-1", "label": "Gold I", "cost": 50, "prereq": { "all": ["single"] }, "effect": { "startingGoldRank": 1 } },
    { "id": "fire-1", "label": "Fire I", "cost": 50, "prereq": { "all": ["single"] }, "effect": { "fireRateRank": 1 } }
  ]
}
)json";

	tower::ResearchData loadedResearch()
	{
		tower::ResearchCatalog catalog;
		catalog.loadFromText(kResearchJson, "research.json");
		return catalog.data();
	}
}

TEST_CASE("research catalog start node completed others available", "[tower][research]")
{
	const auto data = loadedResearch();
	const auto states = tower::evaluateGraph(
		tower::researchGraphNodes(data),
		{ "single" });
	CHECK(states.at("single") == tower::GraphNodeState::Completed);
	CHECK(states.at("double") == tower::GraphNodeState::Available);
	CHECK(states.at("gold-1") == tower::GraphNodeState::Available);
	CHECK(states.at("fire-1") == tower::GraphNodeState::Available);
}

TEST_CASE("tryResearch unlocks double and persists", "[tower][research]")
{
	const auto path = uniqueSavePath();
	writeText(
		path,
		R"json({ "version": 1, "currency": { "points": 100 }, "research": ["single"] })json");

	const auto data = loadedResearch();
	tower::ProfileService profile;
	profile.load(path.string(), false);
	profile.syncFromResearch(data);
	REQUIRE(profile.tryResearch("double", data));
	CHECK(profile.profile().points == 0);
	CHECK(profile.profile().researched.contains("double"));
	CHECK(profile.profile().ownedTowers == std::vector<std::string>{ "single", "double" });
	CHECK_FALSE(profile.tryResearch("double", data));
	CHECK(profile.profile().points == 0);

	tower::ProfileService reader;
	reader.load(path.string(), false);
	reader.syncFromResearch(data);
	CHECK(reader.profile().researched.contains("double"));
	CHECK(reader.profile().ownedTowers == std::vector<std::string>{ "single", "double" });
	std::filesystem::remove_all(path.parent_path());
}

TEST_CASE("tryResearch fails without points", "[tower][research]")
{
	const auto path = uniqueSavePath();
	const auto data = loadedResearch();
	tower::ProfileService profile;
	profile.load(path.string(), false);
	profile.syncFromResearch(data);
	CHECK_FALSE(profile.tryResearch("double", data));
	CHECK_FALSE(profile.profile().researched.contains("double"));
	std::filesystem::remove_all(path.parent_path());
}

TEST_CASE("tryResearch gold-1 locked without its prereq", "[tower][research]")
{
	const auto path = uniqueSavePath();
	writeText(
		path,
		R"json({ "version": 1, "currency": { "points": 50 }, "research": ["other"] })json");

	tower::ResearchCatalog catalog;
	catalog.loadFromText(
		R"json({
  "version": 1,
  "nodes": [
    { "id": "root", "label": "Root", "start": false, "cost": 0 },
    { "id": "gold-1", "label": "Gold I", "cost": 50, "prereq": { "all": ["root"] }, "effect": { "startingGoldRank": 1 } }
  ]
})json",
		"research.json");

	tower::ProfileService profile;
	profile.load(path.string(), false);
	CHECK_FALSE(profile.tryResearch("gold-1", catalog.data()));
	CHECK_FALSE(profile.profile().researched.contains("gold-1"));
	std::filesystem::remove_all(path.parent_path());
}
