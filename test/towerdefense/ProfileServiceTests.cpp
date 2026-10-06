#include <catch2/catch_test_macros.hpp>
#include <Services/ProfileService.hpp>
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
			/ ("helsinki-td-profile-" + std::to_string(stamp));
		std::filesystem::create_directories(dir);
		return dir / "save.json";
	}

	void writeText(const std::filesystem::path& path, const std::string& text)
	{
		std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
		file << text;
	}
}

TEST_CASE("missing save creates defaults", "[tower][profile]")
{
	const auto path = uniqueSavePath();
	tower::ProfileService profile;
	profile.load(path.string(), false);
	CHECK(profile.progress().cleared.empty());
	CHECK(profile.progress().skipped.empty());
	CHECK(profile.profile().ownedTowers == std::vector<std::string>{ "single" });
	REQUIRE(std::filesystem::exists(path));
	std::filesystem::remove_all(path.parent_path());
}

TEST_CASE("valid save loads cleared node", "[tower][profile]")
{
	const auto path = uniqueSavePath();
	writeText(
		path,
		R"json({
  "version": 1,
  "cleared": { "n1": {} },
  "skipped": ["n2"],
  "currency": { "points": 50 },
  "unlocks": { "towers": ["single"] },
  "upgrades": { "startingGold": 1, "fireRate": 2 }
})json");

	tower::ProfileService profile;
	profile.load(path.string(), false);
	CHECK(profile.progress().cleared.contains("n1"));
	CHECK(profile.progress().skipped.contains("n2"));
	CHECK(profile.profile().points == 50);
	CHECK(profile.profile().startingGoldRank == 1);
	CHECK(profile.profile().fireRateRank == 2);
	std::filesystem::remove_all(path.parent_path());
}

TEST_CASE("garbage save uses defaults and does not throw", "[tower][profile]")
{
	const auto path = uniqueSavePath();
	writeText(path, "{ this is not json");

	tower::ProfileService profile;
	CHECK_NOTHROW(profile.load(path.string(), false));
	CHECK(profile.progress().cleared.empty());
	CHECK(profile.progress().skipped.empty());
	std::filesystem::remove_all(path.parent_path());
}

TEST_CASE("save then load round-trip", "[tower][profile]")
{
	const auto path = uniqueSavePath();
	tower::ProfileService writer;
	writer.load(path.string(), false);
	writer.progress().cleared.insert("level-1");
	writer.progress().skipped.insert("side");
	writer.save();

	tower::ProfileService reader;
	reader.load(path.string(), false);
	CHECK(reader.progress().cleared.contains("level-1"));
	CHECK(reader.progress().skipped.contains("side"));
	std::filesystem::remove_all(path.parent_path());
}

TEST_CASE("resetOnBoot wipes a seeded save", "[tower][profile]")
{
	const auto path = uniqueSavePath();
	writeText(
		path,
		R"json({ "version": 1, "cleared": { "n1": {} }, "skipped": [] })json");

	tower::ProfileService profile;
	profile.load(path.string(), true);
	CHECK(profile.progress().cleared.empty());
	CHECK(profile.progress().skipped.empty());
	REQUIRE(std::filesystem::exists(path));
	std::filesystem::remove_all(path.parent_path());
}
