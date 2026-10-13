#include <World/GameCommand.hpp>
#include <World/ShipDesign.hpp>
#include <World/StarterMap.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace ShipDesignTests
{

	ShipClassDraft legalSmallDraft(const char* name)
	{
		return ShipClassDraft{
			.name = name,
			.hullSize = HullSize::Small,
			.parts = {
				.reactorCount = 1,
				.engineCount = 1,
				.laserCount = 2,
				.missileTubeCount = 2,
				.cargoCount = 2 } };
	}

	const ShipClass* classNamed(const WorldData& world, const char* name)
	{
		for (const auto& shipClass : world.shipClasses)
		{
			if (shipClass.name == name && shipClass.empireId == HelionEmpireId)
			{
				return &shipClass;
			}
		}

		return nullptr;
	}

TEST_CASE("empty name is a draft fault", "[slr][ShipDesign]")
{
	ShipClassDraft draft = legalSmallDraft("Nike");
	draft.name.clear();
	REQUIRE(validateShipClassDraft(draft) == ShipClassDraftFault::EmptyName);
}

TEST_CASE("draft without an engine is a fault", "[slr][ShipDesign]")
{
	ShipClassDraft draft = legalSmallDraft("Scout");
	draft.parts.engineCount = 0;
	REQUIRE(validateShipClassDraft(draft) == ShipClassDraftFault::NoEngine);
}

TEST_CASE("over-slot draft is a fault", "[slr][ShipDesign]")
{
	ShipClassDraft draft = legalSmallDraft("Packed");
	draft.parts.cargoCount = 20;
	REQUIRE(validateShipClassDraft(draft) == ShipClassDraftFault::OverSlots);
}

TEST_CASE("non-positive power is a fault", "[slr][ShipDesign]")
{
	ShipClassDraft draft = legalSmallDraft("Hungry");
	draft.parts.reactorCount = 0;
	REQUIRE(validateShipClassDraft(draft) == ShipClassDraftFault::PowerNotPositive);
}

TEST_CASE("legal small draft has no fault", "[slr][ShipDesign]")
{
	REQUIRE(validateShipClassDraft(legalSmallDraft("Nike")) == ShipClassDraftFault::None);
}

TEST_CASE("create ship class writes a new derived warship", "[slr][ShipDesign]")
{
	WorldData world = makeStarterMap();
	const ShipClass* nike = classNamed(world, "Nike");
	REQUIRE(nike != nullptr);
	const long long nikeId = nike->id;
	const long long nikeLasers = nike->laserCount;
	const std::size_t classCount = world.shipClasses.size();
	const long long expectedId = nextShipClassId(world);
	ShipClassDraft draft = legalSmallDraft("Troy");
	draft.parts.laserCount = 1;
	draft.parts.engineCount = 2;
	REQUIRE(execute(world, GameCommand::createShipClass(HelionEmpireId, draft, CommandSource::Player)));
	REQUIRE(world.shipClasses.size() == classCount + 1);
	const ShipClass* troy = world.shipClassById(expectedId);
	REQUIRE(troy != nullptr);
	REQUIRE(troy->name == "Troy");
	REQUIRE(troy->empireId == HelionEmpireId);
	REQUIRE(shipClassHasRole(*troy, HullRole::Warship));
	REQUIRE(troy->hullSize == HullSize::Small);
	REQUIRE(troy->engineCount == 2);
	REQUIRE(troy->laserCount == 1);
	REQUIRE(troy->laserDamage == ShipDesignLaserDamage);
	REQUIRE(troy->closeRate == static_cast<float>(2 * ShipDesignEngineCloseRate));
	REQUIRE(troy->missileTubes == 2);
	REQUIRE(troy->missileDamage == 2 * ShipDesignMissileTubeDamage);
	REQUIRE(troy->magazineSize == 2 * ShipDesignCargoMagazine);
	REQUIRE(troy->maxHull == 28);
	REQUIRE(troy->steelCost == 2000.f + static_cast<GoodAmount>(8 * ShipDesignSteelPerPart));
	REQUIRE(troy->buildTicks == 20);
	const ShipClass* nikeAfter = world.shipClassById(nikeId);
	REQUIRE(nikeAfter != nullptr);
	REQUIRE(nikeAfter->laserCount == nikeLasers);
	REQUIRE(nikeAfter->name == "Nike");
}

TEST_CASE("failed create ship class writes nothing", "[slr][ShipDesign]")
{
	WorldData world = makeStarterMap();
	const std::size_t classCount = world.shipClasses.size();
	ShipClassDraft draft = legalSmallDraft("");
	REQUIRE_FALSE(
		execute(world, GameCommand::createShipClass(HelionEmpireId, draft, CommandSource::Player)));
	REQUIRE(world.shipClasses.size() == classCount);
	REQUIRE(world.empireById(99) == nullptr);
	REQUIRE_FALSE(execute(
		world,
		GameCommand::createShipClass(99, legalSmallDraft("Ghost"), CommandSource::Player)));
	REQUIRE(world.shipClasses.size() == classCount);
}

TEST_CASE("starter Nike and Destroyer match the seed fills", "[slr][ShipDesign]")
{
	const WorldData world = makeStarterMap();
	const ShipClass* nike = classNamed(world, "Nike");
	const ShipClass* destroyer = classNamed(world, "Destroyer");
	REQUIRE(nike != nullptr);
	REQUIRE(destroyer != nullptr);
	REQUIRE(nike->hullSize == HullSize::Small);
	REQUIRE(nike->reactorCount == 1);
	REQUIRE(nike->engineCount == 1);
	REQUIRE(nike->laserCount == 2);
	REQUIRE(nike->missileTubeCount == 2);
	REQUIRE(nike->cargoCount == 2);
	REQUIRE(nike->laserDamage == 8);
	REQUIRE(nike->missileTubes == 2);
	REQUIRE(nike->magazineSize == 80);
	REQUIRE(destroyer->hullSize == HullSize::Small);
	REQUIRE(destroyer->reactorCount == 1);
	REQUIRE(destroyer->engineCount == 2);
	REQUIRE(destroyer->laserCount == 1);
	REQUIRE(destroyer->missileTubeCount == 2);
	REQUIRE(destroyer->cargoCount == 2);
	REQUIRE(destroyer->closeRate == 6.0f);
	REQUIRE(destroyer->laserDamage == 4);
	REQUIRE(validateShipClassDraft(ShipClassDraft{
		.name = nike->name,
		.hullSize = nike->hullSize,
		.parts = {
			.reactorCount = nike->reactorCount,
			.engineCount = nike->engineCount,
			.laserCount = nike->laserCount,
			.missileTubeCount = nike->missileTubeCount,
			.cargoCount = nike->cargoCount } }) == ShipClassDraftFault::None);
}

}
}
