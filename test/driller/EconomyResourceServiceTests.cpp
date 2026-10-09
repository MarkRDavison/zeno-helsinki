#include <catch2/catch_test_macros.hpp>
#include <Services/EconomyResourceService.hpp>
#include <stdexcept>

namespace drl
{
namespace EconomyResourceServiceTests
{

TEST_CASE("set and get resource amounts", "[drl][EconomyResourceService]")
{
	EconomyResourceService economy;
	economy.set(ResourceMoney, 500);
	REQUIRE(economy.get(ResourceMoney) == 500);
	REQUIRE(economy.get(ResourceOre) == 0);
}

TEST_CASE("uncapped max -1 does not clamp", "[drl][EconomyResourceService]")
{
	EconomyResourceService economy;
	economy.setMax(ResourceOre, -1);
	economy.set(ResourceOre, 9999);
	REQUIRE(economy.get(ResourceOre) == 9999);
	REQUIRE(economy.getMax(ResourceOre) == -1);
}

TEST_CASE("finite max clamps set and add", "[drl][EconomyResourceService]")
{
	EconomyResourceService economy;
	economy.setMax(ResourceOre, 10);
	economy.set(ResourceOre, 50);
	REQUIRE(economy.get(ResourceOre) == 10);
	economy.set(ResourceOre, 5);
	economy.add(ResourceOre, 20);
	REQUIRE(economy.get(ResourceOre) == 10);
}

TEST_CASE("canAfford unknown or insufficient is false", "[drl][EconomyResourceService]")
{
	EconomyResourceService economy;
	REQUIRE_FALSE(economy.canAfford(ResourceMoney, 0));
	economy.set(ResourceMoney, 100);
	REQUIRE(economy.canAfford(ResourceMoney, 100));
	REQUIRE_FALSE(economy.canAfford(ResourceMoney, 101));
	REQUIRE_FALSE(economy.canAfford(ResourceOre, 1));
}

TEST_CASE("pay succeeds and refuse overspend", "[drl][EconomyResourceService]")
{
	EconomyResourceService economy;
	economy.set(ResourceMoney, 100);
	REQUIRE(economy.pay(ResourceMoney, 40));
	REQUIRE(economy.get(ResourceMoney) == 60);
	REQUIRE_FALSE(economy.pay(ResourceMoney, 61));
	REQUIRE(economy.get(ResourceMoney) == 60);
	REQUIRE_FALSE(economy.pay("Unknown", 1));
}

TEST_CASE("add ore increases amount", "[drl][EconomyResourceService]")
{
	EconomyResourceService economy;
	economy.setMax(ResourceOre, -1);
	economy.add(ResourceOre, 25);
	economy.add(ResourceOre, 5);
	REQUIRE(economy.get(ResourceOre) == 30);
}

TEST_CASE("registeredHudNames sorts by order not insertion", "[drl][EconomyResourceService]")
{
	EconomyResourceService economy;
	economy.setHud(ResourceMoney, 2, "Money", "cash");
	economy.setHud(ResourceOre, 1, "Ore", "rock");
	const auto names = economy.registeredHudNames();
	REQUIRE(names.size() == 2);
	REQUIRE(names[0] == ResourceOre);
	REQUIRE(names[1] == ResourceMoney);
	REQUIRE(economy.getLabel(ResourceOre) == "Ore");
	REQUIRE(economy.getDescription(ResourceMoney) == "cash");
}

TEST_CASE("duplicate resource HUD order throws", "[drl][EconomyResourceService]")
{
	EconomyResourceService economy;
	economy.setHud(ResourceOre, 1, "Ore", "rock");
	REQUIRE_THROWS_AS(economy.setHud(ResourceMoney, 1, "Money", "cash"), std::invalid_argument);
}

}
}
