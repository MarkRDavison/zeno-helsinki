#include <World/Combat.hpp>
#include <World/DesiredCargo.hpp>
#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace MunitionsTests
{

	std::vector<GoodQuantity> availableCounts(GoodAmount standard, GoodAmount longRange)
	{
		return {
			GoodQuantity{ .goodId = MissileGoodId, .amount = standard },
			GoodQuantity{ .goodId = MissileLongRangeGoodId, .amount = longRange } };
	}

	DesiredCargo longRangeQuotaThenStandardFiller(GoodAmount longQuota)
	{
		DesiredCargo desired;
		desired.rows.push_back(DesiredCargoRow{
			.goodId = MissileLongRangeGoodId,
			.quota = longQuota,
			.fill = DesiredCargoFill::Quota });
		desired.rows.push_back(DesiredCargoRow{
			.goodId = MissileGoodId,
			.quota = 0.f,
			.fill = DesiredCargoFill::FillRemaining });
		return desired;
	}

	TEST_CASE("nextDesiredCargoGoodId reserves quota empties", "[slr][Munitions]")
	{
		const DesiredCargo desired = longRangeQuotaThenStandardFiller(40.f);
		std::vector<GoodQuantity> current;
		std::vector<GoodQuantity> available = availableCounts(100.f, 10.f);
		const GoodAmount capacity = 60.f;
		while (true)
		{
			const long long next = nextDesiredCargoGoodId(desired, current, capacity, available);
			if (next == 0)
			{
				break;
			}

			addMagazineGood(current, next, 1.f);
			REQUIRE(spendMagazineGood(available, next, 1.f));
		}

		REQUIRE(magazineCount(current, MissileLongRangeGoodId) == 10.f);
		REQUIRE(magazineCount(current, MissileGoodId) == 20.f);
		REQUIRE(magazineUsed(current) == 30.f);
	}

	TEST_CASE("quota 0 never picks long-range", "[slr][Munitions]")
	{
		DesiredCargo desired;
		desired.rows.push_back(DesiredCargoRow{
			.goodId = MissileLongRangeGoodId,
			.quota = 0.f,
			.fill = DesiredCargoFill::Quota });
		desired.rows.push_back(DesiredCargoRow{
			.goodId = MissileGoodId,
			.quota = 0.f,
			.fill = DesiredCargoFill::FillRemaining });
		std::vector<GoodQuantity> current;
		const std::vector<GoodQuantity> available = availableCounts(50.f, 50.f);
		REQUIRE(nextDesiredCargoGoodId(desired, current, 60.f, available) == MissileGoodId);
	}

	TEST_CASE("receiving hull rejects a good it does not want", "[slr][Munitions]")
	{
		WorldData world = makeStarterMap();
		REQUIRE(execute(
			world,
			GameCommand::setFleetEscort(1, true, 0, 0, 1, CommandSource::Player)));
		stepGalaxy(world);
		Truck* tender = nullptr;
		for (auto& truck : world.trucks)
		{
			if (truckHasRole(world, truck, HullRole::Tender) && truck.convoyId != 0)
			{
				tender = &truck;
				break;
			}
		}

		REQUIRE(tender != nullptr);
		tender->cargoGoodId = MissileGoodId;
		tender->cargoAmount = 5;
		parkTruckAt(*tender, 3);

		Fleet* home = world.fleetById(1);
		REQUIRE(home != nullptr);
		home->systemId = 3;
		home->motion.route.clear();
		home->motion.progress = 0.0f;
		FleetHull* hull = nullptr;
		long long lowestId = 0;
		for (auto& candidate : home->hulls)
		{
			if (hull == nullptr || candidate.id < lowestId)
			{
				hull = &candidate;
				lowestId = candidate.id;
			}
		}

		REQUIRE(hull != nullptr);
		setFleetClassDesiredCargo(
			*home,
			hull->classId,
			DesiredCargo{
				.rows = {
					DesiredCargoRow{
						.goodId = MissileLongRangeGoodId,
						.quota = 80.f,
						.fill = DesiredCargoFill::Quota } } });
		hull->magazine.clear();
		applyTenderUnloadIntoMagazines(world);
		REQUIRE(magazineUsed(hull->magazine) == 0.f);
		REQUIRE(tender->cargoAmount == 5.f);
	}

	TEST_CASE("combat fires long-range first when both are loaded", "[slr][Munitions]")
	{
		WorldData world;
		world.masterSeed = 1;
		world.empires = {
			Empire{ .id = 1, .name = "A", .defaultPolicyId = 1 },
			Empire{ .id = 2, .name = "B", .defaultPolicyId = 2 },
		};
		world.systems = { StarSystem{ .id = 10, .name = "Arena", .position = { 0.0f, 0.0f } } };
		world.engagementPolicies = {
			EngagementPolicy{
				.id = 1,
				.empireId = 1,
				.name = "Free",
				.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen },
			EngagementPolicy{
				.id = 2,
				.empireId = 2,
				.name = "Hold",
				.ruleOfEngagement = RuleOfEngagement::DoNotFireUnlessFiredUpon },
		};
		world.shipClasses.push_back(ShipClass{
			.id = 1,
			.empireId = 1,
			.name = "Mixed",
			.range = 10.0f,
			.accuracy = 1.0f,
			.laserDamage = 0,
			.maxShields = 0,
			.maxHull = 10,
			.closeRate = 0.0f,
			.hullRoles = hullRoleBit(HullRole::Warship),
			.missileTubes = 1,
			.missileReloadInnerTicks = 10000,
			.magazineSize = 4,
			.missileDamage = 0,
			.missileRange = 200.0f,
			.missileSpeed = 50.0f });
		world.shipClasses.push_back(ShipClass{
			.id = 2,
			.empireId = 2,
			.name = "Target",
			.range = 10.0f,
			.accuracy = 0.0f,
			.laserDamage = 0,
			.maxShields = 0,
			.maxHull = 10,
			.closeRate = 0.0f,
			.hullRoles = hullRoleBit(HullRole::Warship) });
		world.fleets.push_back(Fleet{
			.id = 1,
			.empireId = 1,
			.name = "Shooter",
			.systemId = 10,
			.policyId = 1 });
		appendEmptyMagazineHulls(world, world.fleets.back(), 1, 1);
		addMagazineGood(world.fleets.back().hulls.front().magazine, MissileGoodId, 1.f);
		addMagazineGood(world.fleets.back().hulls.front().magazine, MissileLongRangeGoodId, 1.f);
		world.fleets.push_back(Fleet{
			.id = 2,
			.empireId = 2,
			.name = "Target",
			.systemId = 10,
			.policyId = 2 });
		appendEmptyMagazineHulls(world, world.fleets.back(), 2, 1);
		stepGalaxy(world);
		REQUIRE(magazineCount(world.fleets[0].hulls.front().magazine, MissileLongRangeGoodId) == 0.f);
		REQUIRE(magazineCount(world.fleets[0].hulls.front().magazine, MissileGoodId) == 1.f);
	}

	TEST_CASE("SetDesiredCargo applies to fleet class or standing convoy and drops mismatches", "[slr][Munitions]")
	{
		WorldData world = makeStarterMap();
		constexpr long long nikeClassId = 1;
		constexpr long long destroyerClassId = 2;
		DesiredCargo mixed = longRangeQuotaThenStandardFiller(40.f);
		REQUIRE(execute(
			world,
			GameCommand::setDesiredCargo(
				mixed,
				1,
				DesiredCargoTargetType::FleetMagazine,
				CommandSource::Player,
				nikeClassId)));
		const DesiredCargo homeNike = desiredCargoForFleetClass(*world.fleetById(1), nikeClassId);
		REQUIRE(homeNike.rows.size() == 2);
		REQUIRE(homeNike.rows[0].goodId == MissileLongRangeGoodId);
		const DesiredCargo homeDestroyer =
			desiredCargoForFleetClass(*world.fleetById(1), destroyerClassId);
		REQUIRE(homeDestroyer.rows.size() == 1);
		REQUIRE(homeDestroyer.rows[0].goodId == MissileGoodId);
		REQUIRE(homeDestroyer.rows[0].fill == DesiredCargoFill::FillRemaining);

		const DesiredCargo kestrelNike =
			desiredCargoForFleetClass(*world.fleetById(3), nikeClassId);
		REQUIRE(kestrelNike.rows.size() == 1);
		REQUIRE(kestrelNike.rows[0].goodId == MissileGoodId);
		REQUIRE(kestrelNike.rows[0].fill == DesiredCargoFill::FillRemaining);

		REQUIRE(execute(
			world,
			GameCommand::setFleetEscort(1, true, 0, 0, 1, CommandSource::Player)));
		const long long convoyId = world.fleetById(1)->escortConvoyId;
		REQUIRE(convoyId != 0);
		REQUIRE(execute(
			world,
			GameCommand::setDesiredCargo(
				mixed,
				convoyId,
				DesiredCargoTargetType::StandingConvoyHold,
				CommandSource::Player)));
		REQUIRE(world.convoyById(convoyId)->holdCargo.rows[0].goodId == MissileLongRangeGoodId);

		REQUIRE_FALSE(execute(
			world,
			GameCommand::setDesiredCargo(
				mixed,
				999,
				DesiredCargoTargetType::FleetMagazine,
				CommandSource::Player,
				nikeClassId)));
		REQUIRE_FALSE(execute(
			world,
			GameCommand::setDesiredCargo(
				mixed,
				1,
				DesiredCargoTargetType::StandingConvoyHold,
				CommandSource::Player)));
		REQUIRE_FALSE(execute(
			world,
			GameCommand::setDesiredCargo(
				mixed,
				1,
				DesiredCargoTargetType::FleetMagazine,
				CommandSource::Player,
				0)));
		REQUIRE_FALSE(execute(
			world,
			GameCommand::setDesiredCargo(
				mixed,
				1,
				DesiredCargoTargetType::FleetMagazine,
				CommandSource::Player,
				HelionCivilianTruckClassId)));

		DesiredCargo twoFillers;
		twoFillers.rows.push_back(DesiredCargoRow{
			.goodId = MissileGoodId,
			.fill = DesiredCargoFill::FillRemaining });
		twoFillers.rows.push_back(DesiredCargoRow{
			.goodId = MissileLongRangeGoodId,
			.fill = DesiredCargoFill::FillRemaining });
		REQUIRE_FALSE(execute(
			world,
			GameCommand::setDesiredCargo(
				twoFillers,
				1,
				DesiredCargoTargetType::FleetMagazine,
				CommandSource::Player,
				nikeClassId)));

		DesiredCargo steel;
		steel.rows.push_back(DesiredCargoRow{
			.goodId = SteelGoodId,
			.fill = DesiredCargoFill::FillRemaining });
		REQUIRE_FALSE(execute(
			world,
			GameCommand::setDesiredCargo(
				steel,
				1,
				DesiredCargoTargetType::FleetMagazine,
				CommandSource::Player,
				nikeClassId)));
	}

	FleetHull* hullOfClass(Fleet& fleet, long long classId)
	{
		FleetHull* lowest = nullptr;
		for (auto& hull : fleet.hulls)
		{
			if (hull.classId != classId)
			{
				continue;
			}

			if (lowest == nullptr || hull.id < lowest->id)
			{
				lowest = &hull;
			}
		}

		return lowest;
	}

	TEST_CASE("restock uses each class mix on this fleet only", "[slr][Munitions]")
	{
		WorldData world = makeStarterMap();
		constexpr long long nikeClassId = 1;
		constexpr long long destroyerClassId = 2;
		Fleet* home = world.fleetById(1);
		REQUIRE(home != nullptr);
		FleetHull* nike = hullOfClass(*home, nikeClassId);
		FleetHull* destroyer = hullOfClass(*home, destroyerClassId);
		REQUIRE(nike != nullptr);
		REQUIRE(destroyer != nullptr);
		nike->magazine.clear();
		destroyer->magazine.clear();

		DesiredCargo nikeLongOnly;
		nikeLongOnly.rows.push_back(DesiredCargoRow{
			.goodId = MissileLongRangeGoodId,
			.quota = 20.f,
			.fill = DesiredCargoFill::Quota });
		REQUIRE(execute(
			world,
			GameCommand::setDesiredCargo(
				nikeLongOnly,
				home->id,
				DesiredCargoTargetType::FleetMagazine,
				CommandSource::Player,
				nikeClassId)));

		applyYardMagazineRestock(world);
		REQUIRE(magazineCount(nike->magazine, MissileLongRangeGoodId) == 1.f);
		REQUIRE(magazineCount(destroyer->magazine, MissileGoodId) == 1.f);
		REQUIRE(magazineCount(destroyer->magazine, MissileLongRangeGoodId) == 0.f);

		const ShipClass* nikeClass = world.shipClassById(nikeClassId);
		REQUIRE(nikeClass != nullptr);
		for (long long i = 0; i < nikeClass->magazineSize + 5; ++i)
		{
			applyYardMagazineRestock(world);
		}

		REQUIRE(magazineCount(nike->magazine, MissileLongRangeGoodId) == 20.f);
		REQUIRE(magazineUsed(nike->magazine) == 20.f);
		REQUIRE(magazineUsed(nike->magazine) <= static_cast<GoodAmount>(nikeClass->magazineSize));
	}

}
}
