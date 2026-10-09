#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <stdexcept>

namespace drl
{
namespace TerrainAlterationServiceTests
{

struct Fixture
{
	TerrainData data;
	TerrainAlterationService service{ data };
};

TEST_CASE("digShaft level 0 succeeds from initial shaft", "[drl][TerrainAlterationService]")
{
	Fixture f;
	REQUIRE(f.data.shaftLevel == -1);
	REQUIRE(f.service.digShaft(0));
	REQUIRE(f.data.shaftLevel == 0);
}

TEST_CASE("digShaft skip-level fails", "[drl][TerrainAlterationService]")
{
	Fixture f;
	REQUIRE_FALSE(f.service.digShaft(1));
	REQUIRE(f.data.shaftLevel == -1);
	REQUIRE(f.service.digShaft(0));
	REQUIRE_FALSE(f.service.digShaft(2));
	REQUIRE(f.data.shaftLevel == 0);
}

TEST_CASE("digShaft creates no side tiles", "[drl][TerrainAlterationService]")
{
	Fixture f;
	REQUIRE(f.service.digShaft(0));
	REQUIRE_FALSE(f.service.doesTileExist(0, 1));
	REQUIRE_FALSE(f.service.doesTileExist(0, -1));
	REQUIRE_FALSE(f.service.isTileDugOut(0, 1));
	REQUIRE_FALSE(f.service.isTileDugOut(0, -1));
}

TEST_CASE("digTile column 0 fails", "[drl][TerrainAlterationService]")
{
	Fixture f;
	REQUIRE(f.service.digShaft(0));
	REQUIRE_FALSE(f.service.digTile(0, 0));
}

TEST_CASE("digTile re-dig fails left and right", "[drl][TerrainAlterationService]")
{
	Fixture f;
	REQUIRE(f.service.digShaft(0));
	REQUIRE(f.service.digTile(0, 1));
	REQUIRE_FALSE(f.service.digTile(0, 1));
	REQUIRE(f.service.digTile(0, -1));
	REQUIRE_FALSE(f.service.digTile(0, -1));
}

TEST_CASE("digTile contiguous next tile ok left and right", "[drl][TerrainAlterationService]")
{
	Fixture f;
	REQUIRE(f.service.digShaft(0));
	REQUIRE(f.service.digTile(0, 1));
	REQUIRE(f.service.digTile(0, 2));
	REQUIRE(f.service.isTileDugOut(0, 1));
	REQUIRE(f.service.isTileDugOut(0, 2));
	REQUIRE(f.service.digTile(0, -1));
	REQUIRE(f.service.digTile(0, -2));
	REQUIRE(f.service.isTileDugOut(0, -1));
	REQUIRE(f.service.isTileDugOut(0, -2));
}

TEST_CASE("digTile gap of undug tiles fails left and right", "[drl][TerrainAlterationService]")
{
	Fixture f;
	REQUIRE(f.service.digShaft(0));
	REQUIRE(f.service.digTile(0, 1));
	REQUIRE_FALSE(f.service.digTile(0, 3));
	REQUIRE(f.service.digTile(0, -1));
	REQUIRE_FALSE(f.service.digTile(0, -3));
}

TEST_CASE("doesTileExist getTile initialiseTile left and right", "[drl][TerrainAlterationService]")
{
	Fixture f;
	REQUIRE(f.service.digShaft(0));
	REQUIRE_FALSE(f.service.doesTileExist(0, 1));
	REQUIRE_FALSE(f.service.doesTileExist(0, -1));

	f.service.initialiseTile(0, 1);
	f.service.initialiseTile(0, -1);
	REQUIRE(f.service.doesTileExist(0, 1));
	REQUIRE(f.service.doesTileExist(0, -1));
	REQUIRE_FALSE(f.service.isTileDugOut(0, 1));
	REQUIRE_FALSE(f.service.isTileDugOut(0, -1));

	auto& right = f.service.getTile(0, 1);
	auto& left = f.service.getTile(0, -1);
	REQUIRE_FALSE(right.dugOut);
	REQUIRE_FALSE(left.dugOut);
	right.dugOut = true;
	left.dugOut = true;
	REQUIRE(f.service.getTile(0, 1).dugOut);
	REQUIRE(f.service.getTile(0, -1).dugOut);
	REQUIRE(f.service.isTileDugOut(0, 1));
	REQUIRE(f.service.isTileDugOut(0, -1));

	const auto& constService = f.service;
	REQUIRE(constService.getTile(0, 1).dugOut);
	REQUIRE(constService.getTile(0, -1).dugOut);
}

TEST_CASE("getTile shaft throws", "[drl][TerrainAlterationService]")
{
	Fixture f;
	REQUIRE(f.service.digShaft(0));
	REQUIRE_THROWS_AS(f.service.getTile(0, 0), std::runtime_error);
}

TEST_CASE("doesLevelExist tracks shaft", "[drl][TerrainAlterationService]")
{
	Fixture f;
	REQUIRE_FALSE(f.service.doesLevelExist(0));
	REQUIRE(f.service.doesLevelExist(-1));
	REQUIRE(f.service.digShaft(0));
	REQUIRE(f.service.doesLevelExist(0));
	REQUIRE_FALSE(f.service.doesLevelExist(1));
}

TEST_CASE("canTileBeReached surface", "[drl][TerrainAlterationService]")
{
	Fixture f;
	REQUIRE(f.service.canTileBeReached(-1, 0));
	REQUIRE(f.service.canTileBeReached(-1, 4));
	REQUIRE_FALSE(f.service.canTileBeReached(0, 1));
	REQUIRE(f.service.digShaft(0));
	REQUIRE(f.service.canTileBeReached(0, 0));
	REQUIRE_FALSE(f.service.canTileBeReached(0, 1));
	REQUIRE(f.service.digTile(0, 1));
	REQUIRE(f.service.canTileBeReached(0, 1));
}

TEST_CASE("isLevelNextShaftLevel true only for shaftLevel+1", "[drl][TerrainAlterationService]")
{
	Fixture f;
	REQUIRE(f.service.isLevelNextShaftLevel(0));
	REQUIRE_FALSE(f.service.isLevelNextShaftLevel(1));
	REQUIRE_FALSE(f.service.isLevelNextShaftLevel(-1));
	REQUIRE(f.service.digShaft(0));
	REQUIRE(f.service.isLevelNextShaftLevel(1));
	REQUIRE_FALSE(f.service.isLevelNextShaftLevel(0));
	REQUIRE_FALSE(f.service.isLevelNextShaftLevel(2));
}

}
}
