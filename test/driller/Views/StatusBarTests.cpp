#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/UpgradeData.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/UpgradeService.hpp>
#include <Views/StatusBarChips.hpp>

namespace drl
{
namespace StatusBarTests
{

TEST_CASE("chips are resources by order then upgrades by order", "[drl][StatusBar]")
{
	EconomyResourceService economy;
	UpgradeData upgradeData;
	UpgradeService upgrades{ upgradeData };

	economy.setHud(ResourceMoney, 10, "Money", "cash");
	economy.setHud(ResourceOre, 20, "Ore", "rock");
	upgrades.registerHud(UpgradeRefine, 1, "Ore multiplier", "bonus");

	const auto chips = collectStatusBarChips(economy, upgrades);
	REQUIRE(chips.size() == 3);
	REQUIRE(chips[0].name == ResourceMoney);
	REQUIRE(chips[0].kind == StatusChipKind::Resource);
	REQUIRE(chips[1].name == ResourceOre);
	REQUIRE(chips[1].kind == StatusChipKind::Resource);
	REQUIRE(chips[2].name == UpgradeRefine);
	REQUIRE(chips[2].kind == StatusChipKind::Upgrade);
}

TEST_CASE("tooltip is label value and description", "[drl][StatusBar]")
{
	REQUIRE(formatStatusTooltip("Ore", "15", "Mined from underground tiles.")
		== "Ore: 15\nMined from underground tiles.");
}

TEST_CASE("status values use economy amounts and ore multiplier", "[drl][StatusBar]")
{
	EconomyResourceService economy;
	UpgradeData upgradeData;
	UpgradeService upgrades{ upgradeData };
	economy.set(ResourceOre, 15);
	economy.setHud(ResourceOre, 1, "Ore", "rock");
	upgrades.registerHud(UpgradeRefine, 1, "Ore multiplier", "bonus");
	upgradeData.oreMultiplier = 1.5f;

	const auto chips = collectStatusBarChips(economy, upgrades);
	REQUIRE(formatStatusValue(chips[0], economy, upgrades) == "15");
	REQUIRE(formatStatusValue(chips[1], economy, upgrades) == "1.500");
}

}
}
