#include <Views/GalaxyDrawConstants.hpp>
#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <World/PickFleet.hpp>
#include <World/PickTruck.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <World/WorldData.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace TenderTests
{

	Truck* firstTender(WorldData& world)
	{
		for (auto& truck : world.trucks)
		{
			if (truckHasRole(world, truck, HullRole::Tender))
			{
				return &truck;
			}
		}

		return nullptr;
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

	TEST_CASE("starter seeds two Helion tenders, not extra trucks of cargo classes", "[slr][Tender]")
	{
		WorldData world = makeStarterMap();
		long long tenders = 0;
		for (const auto& truck : world.trucks)
		{
			if (truckHasRole(world, truck, HullRole::Tender))
			{
				++tenders;
				REQUIRE(truck.classId == HelionTenderClassId);
				REQUIRE(isTruckAtSystem(truck, 1));
				REQUIRE(truck.cargoAmount == 0);
				REQUIRE_FALSE(truckHasCargoHaulRole(world, truck));
			}
		}

		REQUIRE(tenders == StarterTenderCount);
	}

	TEST_CASE("escort starts at zero seats and joins nothing", "[slr][Tender]")
	{
		WorldData world = makeStarterMap();
		REQUIRE(execute(
			world,
			GameCommand::setFleetEscort(1, true, 0, 0, 0, CommandSource::Player)));
		const Fleet* home = world.fleetById(1);
		REQUIRE(home != nullptr);
		REQUIRE(home->escortConvoyId != 0);
		const Convoy* standing = world.convoyById(home->escortConvoyId);
		REQUIRE(standing != nullptr);
		REQUIRE(standing->desiredCivilianTruckCount == 0);
		REQUIRE(standing->desiredMilitaryTruckCount == 0);
		REQUIRE(standing->desiredTenderCount == 0);

		stepGalaxy(world);
		REQUIRE(countTendersInConvoy(world, standing->id) == 0);
		REQUIRE(countCivilianTrucksInConvoy(world, standing->id) == 0);
		REQUIRE(countMilitaryTrucksInConvoy(world, standing->id) == 0);
	}

	TEST_CASE("raising tender seats joins tenders; they refuse steel jobs", "[slr][Tender]")
	{
		WorldData world = makeStarterMap();
		REQUIRE(execute(
			world,
			GameCommand::setFleetEscort(1, true, 0, 0, 2, CommandSource::Player)));
		stepGalaxy(world);
		const Fleet* home = world.fleetById(1);
		REQUIRE(home != nullptr);
		REQUIRE(countTendersInConvoy(world, home->escortConvoyId) == 2);

		Truck* tender = firstTender(world);
		REQUIRE(tender != nullptr);
		const HaulJob* steel = haulJobFor(world, 1, SteelGoodId);
		REQUIRE(steel != nullptr);
		REQUIRE_FALSE(execute(
			world,
			GameCommand::acceptHaulJob(tender->id, steel->id, CommandSource::Ai)));
	}

	TEST_CASE("parked tender at a yard loads one missile per tick", "[slr][Tender]")
	{
		WorldData world = makeStarterMap();
		REQUIRE(execute(
			world,
			GameCommand::setFleetEscort(1, true, 0, 0, 1, CommandSource::Player)));
		stepGalaxy(world);
		Truck* tender = firstTender(world);
		REQUIRE(tender != nullptr);
		REQUIRE(tender->convoyId != 0);
		const StarSystem* helion = world.systemById(1);
		REQUIRE(helion != nullptr);
		const long long pileBefore = getStockpileAmount(*helion, MissileGoodId);

		stepGalaxy(world);
		REQUIRE(tender->cargoGoodId == MissileGoodId);
		REQUIRE(tender->cargoAmount == 1);
		REQUIRE(getStockpileAmount(*helion, MissileGoodId) == pileBefore - 1);
	}

	TEST_CASE("tender fills to capacity then stops", "[slr][Tender]")
	{
		WorldData world = makeStarterMap();
		REQUIRE(execute(
			world,
			GameCommand::setFleetEscort(1, true, 0, 0, 1, CommandSource::Player)));
		stepGalaxy(world);
		Truck* tender = firstTender(world);
		REQUIRE(tender != nullptr);
		const StarSystem* helion = world.systemById(1);
		REQUIRE(helion != nullptr);
		const long long pileBefore = getStockpileAmount(*helion, MissileGoodId);

		for (long long i = 0; i < TruckCapacity_Missiles; ++i)
		{
			stepGalaxy(world);
		}

		REQUIRE(tender->cargoAmount == TruckCapacity_Missiles);
		REQUIRE(getStockpileAmount(*helion, MissileGoodId) == pileBefore - TruckCapacity_Missiles);

		stepGalaxy(world);
		REQUIRE(tender->cargoAmount == TruckCapacity_Missiles);
		REQUIRE(getStockpileAmount(*helion, MissileGoodId) == pileBefore - TruckCapacity_Missiles);
	}

	TEST_CASE("loaded tenders follow the escort fleet off the yard", "[slr][Tender]")
	{
		WorldData world = makeStarterMap();
		Fleet* kestrel = world.fleetById(3);
		REQUIRE(kestrel != nullptr);
		kestrel->systemId = 1;
		kestrel->motion.route.clear();
		kestrel->motion.progress = 0.0f;
		REQUIRE(execute(
			world,
			GameCommand::setFleetEscort(3, true, 0, 0, 2, CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(countTendersInConvoy(world, kestrel->escortConvoyId) == 2);

		for (auto& truck : world.trucks)
		{
			if (truck.convoyId == kestrel->escortConvoyId)
			{
				truck.cargoGoodId = MissileGoodId;
				truck.cargoAmount = 10;
			}
		}

		REQUIRE(execute(world, GameCommand::moveFleetToSystem(3, 3, CommandSource::Player)));
		stepGalaxy(world);
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId != kestrel->escortConvoyId)
			{
				continue;
			}

			REQUIRE(truck.cargoAmount >= 10);
			REQUIRE_FALSE(truck.motion.route.empty());
			REQUIRE(truck.motion.route.back() == 3);
		}
	}

	TEST_CASE("standing escort and tenders hop at the slowest member speed", "[slr][Tender]")
	{
		WorldData world = makeStarterMap();
		Fleet* kestrel = world.fleetById(3);
		REQUIRE(kestrel != nullptr);
		kestrel->systemId = 1;
		kestrel->motion.route.clear();
		kestrel->motion.progress = 0.0f;
		kestrel->motion.speed = 50.0f;
		REQUIRE(execute(
			world,
			GameCommand::setFleetEscort(3, true, 0, 0, 2, CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(countTendersInConvoy(world, kestrel->escortConvoyId) == 2);
		for (auto& truck : world.trucks)
		{
			if (truck.convoyId == kestrel->escortConvoyId)
			{
				truck.motion.speed = 25.0f;
			}
		}

		REQUIRE(execute(world, GameCommand::moveFleetToSystem(3, 3, CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(kestrel->motion.speed == Catch::Approx(50.0f));
		REQUIRE(kestrel->motion.route.size() >= 2);
		const StarSystem* from = world.systemById(kestrel->motion.route[0]);
		const StarSystem* to = world.systemById(kestrel->motion.route[1]);
		REQUIRE(from != nullptr);
		REQUIRE(to != nullptr);
		const float length = glm::length(to->position - from->position);
		REQUIRE(length > 0.0f);
		REQUIRE(kestrel->motion.progress == Catch::Approx(25.0f / length));
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId != kestrel->escortConvoyId)
			{
				continue;
			}

			REQUIRE(truck.motion.speed == Catch::Approx(25.0f));
			REQUIRE(truck.motion.hop == kestrel->motion.hop);
			REQUIRE(truck.motion.progress == Catch::Approx(kestrel->motion.progress));
			REQUIRE(truck.motion.route == kestrel->motion.route);
		}
	}

	TEST_CASE("standing escort tenders trail the fleet marker like haul trucks", "[slr][Tender]")
	{
		WorldData world = makeStarterMap();
		Fleet* kestrel = world.fleetById(3);
		REQUIRE(kestrel != nullptr);
		kestrel->systemId = 1;
		kestrel->motion.route.clear();
		kestrel->motion.progress = 0.0f;
		REQUIRE(execute(
			world,
			GameCommand::setFleetEscort(3, true, 0, 0, 2, CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(execute(world, GameCommand::moveFleetToSystem(3, 3, CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(kestrel->motion.progress > 0.0f);
		REQUIRE(kestrel->motion.progress < 1.0f);

		const auto fleetPos = fleetMarkerPosition(world, *kestrel);
		REQUIRE(fleetPos.has_value());
		const auto onLane = laneWorldPosition(world, kestrel->motion);
		REQUIRE(onLane.has_value());
		long long fromId = 0;
		long long toId = 0;
		REQUIRE(hopEndpoints(kestrel->motion, fromId, toId));
		const glm::vec2 along = glm::normalize(
			world.systemById(toId)->position - world.systemById(fromId)->position);
		const glm::vec2 expected =
			*onLane - along * (GalaxyFleetSize * 0.5f + GalaxyTruckSize * 0.5f + 4.0f);

		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId != kestrel->escortConvoyId)
			{
				continue;
			}

			const auto truckPos = truckMarkerPosition(world, truck);
			REQUIRE(truckPos.has_value());
			REQUIRE(glm::distance(*truckPos, *fleetPos) > 1.0f);
			REQUIRE(glm::distance(*truckPos, expected) == Catch::Approx(0.0f));
		}
	}

	TEST_CASE("tender still walking to the rally is not copied onto the fleet hop", "[slr][Tender]")
	{
		WorldData world = makeStarterMap();
		Fleet* kestrel = world.fleetById(3);
		REQUIRE(kestrel != nullptr);
		kestrel->systemId = 1;
		kestrel->motion.route.clear();
		kestrel->motion.progress = 0.0f;
		kestrel->motion.speed = 50.0f;
		REQUIRE(execute(
			world,
			GameCommand::setFleetEscort(3, true, 0, 0, 2, CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(countTendersInConvoy(world, kestrel->escortConvoyId) == 2);
		Truck* joiner = nullptr;
		Truck* withFleet = nullptr;
		for (auto& truck : world.trucks)
		{
			if (truck.convoyId != kestrel->escortConvoyId)
			{
				continue;
			}

			truck.motion.speed = 25.0f;
			if (withFleet == nullptr)
			{
				withFleet = &truck;
			}
			else
			{
				joiner = &truck;
			}
		}

		REQUIRE(withFleet != nullptr);
		REQUIRE(joiner != nullptr);
		parkTruckAt(*joiner, 5);
		REQUIRE(execute(world, GameCommand::moveFleetToSystem(3, 3, CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(withFleet->motion.route == kestrel->motion.route);
		REQUIRE(withFleet->motion.hop == kestrel->motion.hop);
		REQUIRE(withFleet->motion.progress == Catch::Approx(kestrel->motion.progress));
		REQUIRE(joiner->motion.route != kestrel->motion.route);
		REQUIRE(joiner->motion.route.back() == 3);
	}

	TEST_CASE("tenders follow the escort onto at-war Ashreach", "[slr][Tender]")
	{
		WorldData world = makeStarterMap();
		declareWarBetweenEmpires(world, HelionEmpireId, AshreachEmpireId);
		Fleet* rival = world.fleetById(2);
		REQUIRE(rival != nullptr);
		rival->hulls.clear();
		rival->composition.clear();
		Fleet* kestrel = world.fleetById(3);
		REQUIRE(kestrel != nullptr);
		kestrel->systemId = 3;
		kestrel->motion.route.clear();
		kestrel->motion.progress = 0.0f;
		REQUIRE(execute(
			world,
			GameCommand::setFleetEscort(3, true, 0, 0, 2, CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(countTendersInConvoy(world, kestrel->escortConvoyId) == 2);

		for (auto& truck : world.trucks)
		{
			if (truck.convoyId != kestrel->escortConvoyId)
			{
				continue;
			}

			truck.cargoGoodId = MissileGoodId;
			truck.cargoAmount = 10;
			parkTruckAt(truck, 3);
		}

		REQUIRE(civilianLanePath(world, 3, 4, HelionEmpireId).empty());
		REQUIRE(execute(world, GameCommand::moveFleetToSystem(3, 4, CommandSource::Player)));
		stepGalaxy(world);
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId != kestrel->escortConvoyId)
			{
				continue;
			}

			REQUIRE_FALSE(truck.motion.route.empty());
			REQUIRE(truck.motion.route.back() == 4);
		}
	}

	TEST_CASE("away from a yard tenders fill friendly magazines from cargo", "[slr][Tender]")
	{
		WorldData world = makeStarterMap();
		REQUIRE(execute(
			world,
			GameCommand::setFleetEscort(1, true, 0, 0, 1, CommandSource::Player)));
		stepGalaxy(world);
		Truck* tender = firstTender(world);
		REQUIRE(tender != nullptr);
		tender->cargoGoodId = MissileGoodId;
		tender->cargoAmount = 5;
		parkTruckAt(*tender, 3);

		Fleet* home = world.fleetById(1);
		REQUIRE(home != nullptr);
		home->systemId = 3;
		home->motion.route.clear();
		home->motion.progress = 0.0f;
		FleetHull* hull = hullByLowestId(*home);
		hull->missileAmmo = 0;

		const StarSystem* helion = world.systemById(1);
		REQUIRE(helion != nullptr);
		const long long pileBefore = getStockpileAmount(*helion, MissileGoodId);

		applyTenderUnloadIntoMagazines(world);
		REQUIRE(hull->missileAmmo == 1);
		REQUIRE(tender->cargoAmount == 4);
		REQUIRE(getStockpileAmount(*helion, MissileGoodId) == pileBefore);
	}

	TEST_CASE("in-transit tender does not unload", "[slr][Tender]")
	{
		WorldData world = makeStarterMap();
		REQUIRE(execute(
			world,
			GameCommand::setFleetEscort(1, true, 0, 0, 1, CommandSource::Player)));
		stepGalaxy(world);
		Truck* tender = firstTender(world);
		REQUIRE(tender != nullptr);
		tender->cargoGoodId = MissileGoodId;
		tender->cargoAmount = 5;
		tender->motion.progress = 0.4f;
		tender->motion.route = { 1, 2 };

		Fleet* home = world.fleetById(1);
		REQUIRE(home != nullptr);
		FleetHull* hull = hullByLowestId(*home);
		hull->missileAmmo = 0;

		applyTenderUnloadIntoMagazines(world);
		REQUIRE(hull->missileAmmo == 0);
		REQUIRE(tender->cargoAmount == 5);
	}

	TEST_CASE("rival fleet does not take tender missiles", "[slr][Tender]")
	{
		WorldData world = makeStarterMap();
		REQUIRE(execute(
			world,
			GameCommand::setFleetEscort(1, true, 0, 0, 1, CommandSource::Player)));
		stepGalaxy(world);
		Truck* tender = firstTender(world);
		REQUIRE(tender != nullptr);
		tender->cargoGoodId = MissileGoodId;
		tender->cargoAmount = 5;
		parkTruckAt(*tender, 3);

		Fleet* home = world.fleetById(1);
		REQUIRE(home != nullptr);
		home->systemId = 3;
		home->motion.progress = 0.0f;
		home->motion.route.clear();

		Fleet* kestrel = world.fleetById(2);
		REQUIRE(kestrel != nullptr);
		kestrel->systemId = 3;
		kestrel->motion.progress = 0.0f;
		kestrel->motion.route.clear();
		FleetHull* rivalHull = hullByLowestId(*kestrel);
		rivalHull->missileAmmo = 0;

		applyTenderUnloadIntoMagazines(world);
		REQUIRE(rivalHull->missileAmmo == 0);
		REQUIRE(tender->cargoAmount == 5);
	}

	TEST_CASE("MilitaryCargo combined with Tender hauls and loads missiles", "[slr][Tender]")
	{
		WorldData world = makeStarterMap();
		constexpr long long comboClassId = 99;
		world.shipClasses.push_back(ShipClass{
			.id = comboClassId,
			.empireId = HelionEmpireId,
			.name = "Armed tender",
			.hullRoles = hullRoleBit(HullRole::MilitaryCargo) | hullRoleBit(HullRole::Tender) });
		Truck& combo = world.trucks.emplace_back();
		combo.id = 900;
		combo.empireId = HelionEmpireId;
		combo.classId = comboClassId;
		combo.motion = { .route = { 1 }, .speed = 25.0f };
		REQUIRE(truckHasRole(world, combo, HullRole::MilitaryCargo));
		REQUIRE(truckHasRole(world, combo, HullRole::Tender));

		StarSystem* junction = world.systemById(3);
		REQUIRE(junction != nullptr);
		for (auto& stock : junction->stocks)
		{
			if (stock.goodId == SteelGoodId)
			{
				stock.goodAmount = TruckCapacity_Steel;
			}
		}

		const HaulJob* steel = haulJobFor(world, 1, SteelGoodId);
		REQUIRE(steel != nullptr);
		REQUIRE(execute(world, GameCommand::acceptHaulJob(combo.id, steel->id, CommandSource::Ai)));
		REQUIRE(combo.haulJobId == steel->id);

		combo.haulJobId = 0;
		combo.convoyId = 0;
		combo.cargoAmount = 0;
		combo.cargoGoodId = 0;
		parkTruckAt(combo, 1);
		std::erase_if(world.trucks, [](const Truck& truck) {
			return truck.id != 900 && truck.classId == HelionTenderClassId;
		});
		Truck* comboAfter = world.truckById(900);
		REQUIRE(comboAfter != nullptr);

		REQUIRE(execute(
			world,
			GameCommand::setFleetEscort(1, true, 0, 0, 1, CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(comboAfter->convoyId != 0);

		const StarSystem* helion = world.systemById(1);
		REQUIRE(helion != nullptr);
		const long long pileBefore = getStockpileAmount(*helion, MissileGoodId);
		stepGalaxy(world);
		REQUIRE(comboAfter->cargoAmount == 1);
		REQUIRE(getStockpileAmount(*helion, MissileGoodId) == pileBefore - 1);
	}

}
}
