#include <catch2/catch_test_macros.hpp>
#include <Core/PlayerDig.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/TerrainAlterationService.hpp>

using drl::EconomyResourceService;
using drl::ResourceMoney;
using drl::TerrainAlterationService;
using drl::TerrainData;
using drl::tryPlayerDigShaft;

namespace
{
	struct Fixture
	{
		TerrainData data;
		TerrainAlterationService terrain{ data };
		EconomyResourceService economy;
	};
}

TEST_CASE("tryPlayerDigShaft skip-level does not pay", "[drl][tryPlayerDigShaft]")
{
	Fixture f;
	f.economy.setMax(ResourceMoney, -1);
	f.economy.set(ResourceMoney, 500);
	REQUIRE_FALSE(tryPlayerDigShaft(f.terrain, f.economy, 1));
	REQUIRE(f.data.shaftLevel == -1);
	REQUIRE(f.economy.get(ResourceMoney) == 500);
}

TEST_CASE("tryPlayerDigShaft cannot afford does not dig", "[drl][tryPlayerDigShaft]")
{
	Fixture f;
	REQUIRE(f.terrain.digShaft(0));
	f.economy.setMax(ResourceMoney, -1);
	f.economy.set(ResourceMoney, 50);
	REQUIRE_FALSE(tryPlayerDigShaft(f.terrain, f.economy, 1));
	REQUIRE(f.data.shaftLevel == 0);
	REQUIRE(f.economy.get(ResourceMoney) == 50);
}

TEST_CASE("tryPlayerDigShaft pays 100 times level then digs", "[drl][tryPlayerDigShaft]")
{
	Fixture f;
	REQUIRE(f.terrain.digShaft(0));
	f.economy.setMax(ResourceMoney, -1);
	f.economy.set(ResourceMoney, 500);
	REQUIRE(tryPlayerDigShaft(f.terrain, f.economy, 1));
	REQUIRE(f.data.shaftLevel == 1);
	REQUIRE(f.economy.get(ResourceMoney) == 400);
}

TEST_CASE("setup digShaft is free", "[drl][tryPlayerDigShaft]")
{
	Fixture f;
	f.economy.setMax(ResourceMoney, -1);
	f.economy.set(ResourceMoney, 500);
	REQUIRE(f.terrain.digShaft(0));
	REQUIRE(f.terrain.digShaft(1));
	REQUIRE(f.data.shaftLevel == 1);
	REQUIRE(f.economy.get(ResourceMoney) == 500);
}
