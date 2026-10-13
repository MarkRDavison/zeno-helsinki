#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <World/WorldData.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace MagazineRestockTests
{

	void silenceHelionTrafficAndPopulation(WorldData& world)
	{
		world.trucks.clear();
		world.convoys.clear();
		Yard* helionYard = yardAtSystem(world, 1);
		REQUIRE(helionYard != nullptr);
		helionYard->populationConsumePerTick.clear();
		helionYard->desiredPiles.clear();
		helionYard->hullBuildQueue.clear();
		StarSystem* helion = world.systemById(1);
		REQUIRE(helion != nullptr);
		for (auto& stock : helion->stocks)
		{
			if (stock.goodId == SteelGoodId)
			{
				stock.need = 0;
				stock.deferredNeed = 0;
				stock.deferredNeedHeldOneTick = false;
			}
		}

		world.haulJobs.clear();
	}

	FleetHull* hullByLowestId(Fleet& fleet)
	{
		REQUIRE(!fleet.hulls.empty());
		FleetHull* lowest = &fleet.hulls.front();
		for (auto& hull : fleet.hulls)
		{
			if (hull.id < lowest->id)
			{
				lowest = &hull;
			}
		}

		return lowest;
	}

	FleetHull* hullBySecondLowestId(Fleet& fleet)
	{
		REQUIRE(fleet.hulls.size() >= 2);
		FleetHull* lowest = hullByLowestId(fleet);
		FleetHull* second = nullptr;
		for (auto& hull : fleet.hulls)
		{
			if (hull.id == lowest->id)
			{
				continue;
			}

			if (second == nullptr || hull.id < second->id)
			{
				second = &hull;
			}
		}

		REQUIRE(second != nullptr);
		return second;
	}

	void setMissilePile(StarSystem& system, long long amount)
	{
		for (auto& stock : system.stocks)
		{
			if (stock.goodId == MissileGoodId)
			{
				stock.goodAmount = amount;
				stock.inbound = 0;
				stock.need = 0;
				stock.deferredNeed = 0;
				stock.deferredNeedHeldOneTick = false;
				return;
			}
		}

		system.stocks.push_back(GoodStock{ .goodId = MissileGoodId, .goodAmount = amount });
	}

	TEST_CASE("starter yards seed 2000 missiles", "[solar][MagazineRestock]")
	{
		WorldData world = makeStarterMap();
		const StarSystem* helion = world.systemById(1);
		const StarSystem* vesper = world.systemById(8);
		REQUIRE(helion != nullptr);
		REQUIRE(vesper != nullptr);
		REQUIRE(getStockpileAmount(*helion, MissileGoodId) == YardStarterMissileStockpile);
		REQUIRE(getStockpileAmount(*vesper, MissileGoodId) == YardStarterMissileStockpile);
	}

	TEST_CASE("parked Home Fleet hull takes one missile per tick", "[solar][MagazineRestock]")
	{
		WorldData world = makeStarterMap();
		silenceHelionTrafficAndPopulation(world);
		Fleet* home = world.fleetById(1);
		REQUIRE(home != nullptr);
		REQUIRE(fleetIsParked(*home));
		REQUIRE(home->systemId == 1);

		FleetHull* hull = hullByLowestId(*home);
		const ShipClass* shipClass = world.shipClassById(hull->classId);
		REQUIRE(shipClass != nullptr);
		REQUIRE(shipClass->magazineSize > 1);
		hull->missileAmmo = 0;

		const StarSystem* helion = world.systemById(1);
		REQUIRE(helion != nullptr);
		const long long pileBefore = getStockpileAmount(*helion, MissileGoodId);

		stepGalaxy(world);

		REQUIRE(hull->missileAmmo == 1);
		REQUIRE(getStockpileAmount(*helion, MissileGoodId) == pileBefore - 1);
	}

	TEST_CASE("restock fills to magazineSize then stops", "[solar][MagazineRestock]")
	{
		WorldData world = makeStarterMap();
		silenceHelionTrafficAndPopulation(world);
		Fleet* home = world.fleetById(1);
		REQUIRE(home != nullptr);
		FleetHull* hull = hullByLowestId(*home);
		const ShipClass* shipClass = world.shipClassById(hull->classId);
		REQUIRE(shipClass != nullptr);
		REQUIRE(shipClass->magazineSize > 0);
		hull->missileAmmo = 0;

		StarSystem* helion = world.systemById(1);
		REQUIRE(helion != nullptr);
		const long long pileBefore = getStockpileAmount(*helion, MissileGoodId);

		for (long long i = 0; i < shipClass->magazineSize; ++i)
		{
			stepGalaxy(world);
		}

		REQUIRE(hull->missileAmmo == shipClass->magazineSize);
		REQUIRE(getStockpileAmount(*helion, MissileGoodId) == pileBefore - shipClass->magazineSize);

		stepGalaxy(world);
		REQUIRE(hull->missileAmmo == shipClass->magazineSize);
		REQUIRE(getStockpileAmount(*helion, MissileGoodId) == pileBefore - shipClass->magazineSize);
	}

	TEST_CASE("thin pile fills lower hull ids first", "[solar][MagazineRestock]")
	{
		WorldData world = makeStarterMap();
		silenceHelionTrafficAndPopulation(world);
		Fleet* home = world.fleetById(1);
		REQUIRE(home != nullptr);
		FleetHull* first = hullByLowestId(*home);
		FleetHull* second = hullBySecondLowestId(*home);
		first->missileAmmo = 0;
		second->missileAmmo = 0;

		StarSystem* helion = world.systemById(1);
		REQUIRE(helion != nullptr);
		setMissilePile(*helion, 1);

		stepGalaxy(world);
		REQUIRE(first->missileAmmo == 1);
		REQUIRE(second->missileAmmo == 0);
		REQUIRE(getStockpileAmount(*helion, MissileGoodId) == 0);
	}

	TEST_CASE("fleet not at a yard does not restock", "[solar][MagazineRestock]")
	{
		WorldData world = makeStarterMap();
		silenceHelionTrafficAndPopulation(world);
		Fleet* home = world.fleetById(1);
		REQUIRE(home != nullptr);
		home->systemId = 3;
		home->motion.progress = 0.0f;
		home->motion.route.clear();
		FleetHull* hull = hullByLowestId(*home);
		hull->missileAmmo = 0;

		StarSystem* helion = world.systemById(1);
		REQUIRE(helion != nullptr);
		const long long pileBefore = getStockpileAmount(*helion, MissileGoodId);

		stepGalaxy(world);
		REQUIRE(hull->missileAmmo == 0);
		REQUIRE(getStockpileAmount(*helion, MissileGoodId) == pileBefore);
	}

	TEST_CASE("in-transit fleet does not restock", "[solar][MagazineRestock]")
	{
		WorldData world = makeStarterMap();
		silenceHelionTrafficAndPopulation(world);
		Fleet* home = world.fleetById(1);
		REQUIRE(home != nullptr);
		home->motion.progress = 0.4f;
		home->motion.route = { 1, 2 };
		FleetHull* hull = hullByLowestId(*home);
		hull->missileAmmo = 0;

		StarSystem* helion = world.systemById(1);
		REQUIRE(helion != nullptr);
		const long long pileBefore = getStockpileAmount(*helion, MissileGoodId);

		stepGalaxy(world);
		REQUIRE(hull->missileAmmo == 0);
		REQUIRE(getStockpileAmount(*helion, MissileGoodId) == pileBefore);
	}

	TEST_CASE("rival fleet at a yard does not restock", "[solar][MagazineRestock]")
	{
		WorldData world = makeStarterMap();
		silenceHelionTrafficAndPopulation(world);
		Fleet* kestrel = nullptr;
		for (auto& fleet : world.fleets)
		{
			if (fleet.empireId == 2)
			{
				kestrel = &fleet;
				break;
			}
		}

		REQUIRE(kestrel != nullptr);
		kestrel->systemId = 1;
		kestrel->motion.progress = 0.0f;
		kestrel->motion.route.clear();
		FleetHull* hull = hullByLowestId(*kestrel);
		hull->missileAmmo = 0;

		StarSystem* helion = world.systemById(1);
		REQUIRE(helion != nullptr);
		const long long pileBefore = getStockpileAmount(*helion, MissileGoodId);

		applyYardMagazineRestock(world);
		REQUIRE(hull->missileAmmo == 0);
		REQUIRE(getStockpileAmount(*helion, MissileGoodId) == pileBefore);
	}

	TEST_CASE("restock spends missiles not steel", "[solar][MagazineRestock]")
	{
		WorldData world = makeStarterMap();
		silenceHelionTrafficAndPopulation(world);
		StarSystem* helion = world.systemById(1);
		REQUIRE(helion != nullptr);
		for (auto& stock : helion->stocks)
		{
			if (stock.goodId == SteelGoodId)
			{
				stock.goodAmount = YardPopulationReserved_Steel + 50;
			}
		}

		Fleet* home = world.fleetById(1);
		REQUIRE(home != nullptr);
		FleetHull* hull = hullByLowestId(*home);
		hull->missileAmmo = 0;

		const long long steelBefore = getStockpileAmount(*helion, SteelGoodId);
		const long long missilesBefore = getStockpileAmount(*helion, MissileGoodId);
		REQUIRE(steelBefore == YardPopulationReserved_Steel + 50);

		applyYardMagazineRestock(world);

		REQUIRE(hull->missileAmmo == 1);
		REQUIRE(getStockpileAmount(*helion, MissileGoodId) == missilesBefore - 1);
		REQUIRE(getStockpileAmount(*helion, SteelGoodId) == steelBefore);
		REQUIRE(stockpileAvailableForYardConsume(world, 1, SteelGoodId) == 50);
	}

}
}
