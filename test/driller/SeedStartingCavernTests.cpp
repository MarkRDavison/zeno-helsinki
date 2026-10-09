#include <catch2/catch_test_macros.hpp>
#include <Core/SeedStartingCavern.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Services/TerrainAlterationService.hpp>

using drl::seedStartingCavern;
using drl::TerrainAlterationService;
using drl::TerrainData;

namespace
{
	struct Fixture
	{
		TerrainData data;
		TerrainAlterationService service{ data };
	};
}

TEST_CASE("seedStartingCavern digs two shaft levels and a small cavern", "[drl][seed]")
{
	Fixture f;
	seedStartingCavern(f.service);

	REQUIRE(f.data.shaftLevel == 1);

	REQUIRE(f.service.isTileDugOut(0, -2));
	REQUIRE(f.service.isTileDugOut(0, -1));
	REQUIRE(f.service.isTileDugOut(0, 1));
	REQUIRE(f.service.isTileDugOut(0, 2));
	REQUIRE(f.service.isTileDugOut(0, 3));
	REQUIRE(f.service.doesTileExist(0, -3));
	REQUIRE_FALSE(f.service.isTileDugOut(0, -3));
	REQUIRE(f.service.doesTileExist(0, 4));
	REQUIRE_FALSE(f.service.isTileDugOut(0, 4));

	REQUIRE(f.service.isTileDugOut(1, -1));
	REQUIRE(f.service.isTileDugOut(1, 1));
	REQUIRE(f.service.isTileDugOut(1, 2));
	REQUIRE(f.service.doesTileExist(1, -2));
	REQUIRE_FALSE(f.service.isTileDugOut(1, -2));
	REQUIRE(f.service.doesTileExist(1, 3));
	REQUIRE_FALSE(f.service.isTileDugOut(1, 3));
}

TEST_CASE("seedStartingCavern still allows the next shaft level", "[drl][seed]")
{
	Fixture f;
	seedStartingCavern(f.service);
	REQUIRE(f.service.digShaft(2));
	REQUIRE(f.data.shaftLevel == 2);
}
