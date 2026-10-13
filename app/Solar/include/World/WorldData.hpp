#pragma once

#include <World/DesiredCargo.hpp>
#include <World/Goods.hpp>
#include <World/Research.hpp>
#include <helsinki/System/glm.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace slr
{

	constexpr long long HelionEmpireId = 1;
	constexpr long long AshreachEmpireId = 2;
	constexpr long long HelionHoldFirePolicyId = 1;
	constexpr long long AshreachHoldFirePolicyId = 2;
	constexpr long long HelionWeaponsFreePolicyId = 3;
	constexpr long long AshreachWeaponsFreePolicyId = 4;
	constexpr long long HelionHoldRangePolicyId = 5;
	constexpr long long AshreachHoldRangePolicyId = 6;

	enum class RuleOfEngagement
	{
		DoNotFireUnlessFiredUpon,
		FireIfEnemySeen
	};

	enum class FleetApproach
	{
		CloseToLaserRange,
		HoldMissileRange
	};

	enum class SystemOwnershipType
	{
		Empty,
		Extraction,
		Industrial,
		Populated
	};

	enum class HullRole : unsigned
	{
		Warship = 1u << 0,
		CivilianCargo = 1u << 1,
		MilitaryCargo = 1u << 2,
		Tender = 1u << 3
	};

	enum class HullSize
	{
		Small,
		Medium,
		Large
	};

	constexpr unsigned hullRoleBit(HullRole role)
	{
		return static_cast<unsigned>(role);
	}

	struct EngagementPolicy
	{
		long long id{ 0 };
		long long empireId{ 0 };
		std::string name;
		RuleOfEngagement ruleOfEngagement{ RuleOfEngagement::DoNotFireUnlessFiredUpon };
		FleetApproach approach{ FleetApproach::CloseToLaserRange };
	};

	struct Empire
	{
		long long id{ 0 };
		std::string name;
		long long defaultPolicyId{ 0 };
	};

	struct EmpireWar
	{
		long long empireA{ 0 };
		long long empireB{ 0 };
		bool atWar{ false };
	};

	struct CivilianHaulForbid
	{
		long long empireId{ 0 };
		long long forbiddenEmpireId{ 0 };
	};

	struct GoodStock
	{
		long long goodId{ 0 };
		GoodAmount goodAmount{ 0.f };
		GoodAmount inbound{ 0.f };
		GoodAmount need{ 0.f };
		GoodAmount deferredNeed{ 0.f };
		bool deferredNeedHeldOneTick{ false };
	};

	struct StarSystem
	{
		long long id{ 0 };
		std::string name;
		glm::vec2 position{ 0.0f };
		std::vector<GoodStock> stocks;
		long long empireId{ 0 };
	};

	struct Hyperlane
	{
		long long a{ 0 };
		long long b{ 0 };
	};

	struct MineOutput
	{
		long long goodId{ 0 };
		GoodAmount amountPerTick{ 0.f };
		GoodAmount maxStockpile{ 0.f };
	};

	struct Mine
	{
		long long systemId{ 0 };
		std::vector<MineOutput> outputsPerTick;
	};

	struct YardSlip
	{
		long long id{ 0 };
		long long remainingBuildTicks{ 0 };
		long long hullClassId{ 0 };
		long long remainingHullBuildTicks{ 0 };
		long long refitHullId{ 0 };
		long long refitFromClassId{ 0 };
		long long preferredFleetId{ 0 };
	};

	struct YardRefitJob
	{
		long long hullId{ 0 };
		long long fromClassId{ 0 };
		long long toClassId{ 0 };
		long long preferredFleetId{ 0 };
	};

	struct Yard
	{
		long long systemId{ 0 };
		long long empireId{ 0 };
		std::vector<GoodQuantity> desiredPiles;
		std::vector<GoodQuantity> populationConsumePerTick;
		std::vector<GoodQuantity> populationReservedPiles;
		std::vector<YardSlip> slips;
		long long nextSlipConstructionId{ 0 };
		std::vector<long long> hullBuildQueue;
		std::vector<YardRefitJob> hullRefitQueue;
	};

	struct ProductRampSpec
	{
		long long goodId{ 0 };
		long long startEfficiencyPerThousand{ 0 };
		long long maxEfficiencyPerThousand{ FactoryFullEfficiencyPerThousand };
	};

	struct FactoryRecipe
	{
		long long recipeId{ 0 };
		std::vector<GoodQuantity> inputsPerTick;
		long long outputGoodId{ 0 };
		GoodAmount outputAmountPerTick{ 0.f };
		GoodAmount outputMaxStockpile{ 0.f };
		long long ticksPerOutput{ 1 };
	};

	struct Factory
	{
		long long systemId{ 0 };
		long long empireId{ 0 };
		std::vector<GoodQuantity> desiredPiles;
		std::vector<GoodQuantity> inputsPerTick;
		long long outputGoodId{ 0 };
		GoodAmount outputAmountPerTick{ 0.f };
		GoodAmount outputMaxStockpile{ 0.f };
		long long productEfficiencyPerThousand{ FactoryFullEfficiencyPerThousand };
		GoodAmount productRampUnitsProduced{ 0.f };
		long long ticksPerOutput{ 1 };
		long long remainingOutputTicks{ 0 };
		long long recipeId{ 0 };
	};

	struct LaneMotion
	{
		std::vector<long long> route;
		int hop{ 0 };
		float progress{ 0.0f };
		int direction{ 1 };
		float speed{ 0.0f };
	};

	struct Truck
	{
		long long id{ 0 };
		long long empireId{ 0 };
		LaneMotion motion;
		long long cargoGoodId{ 0 };
		// TODO: eventually trucks can transport more than 1 thing at a time
		// i.e. have room for X standard containers
		// and resources can fit Y resource in 1 standard container
		GoodAmount cargoAmount{ 0.f };
		long long haulJobId{ 0 };
		long long convoyId{ 0 };
		long long classId{ 0 };
	};

	struct ShipClass
	{
		long long id{ 0 };
		long long empireId{ 0 };
		std::string name;
		float range{ 0.0f };
		float accuracy{ 0.0f };
		long long laserDamage{ 0 };
		long long maxShields{ 0 };
		long long maxHull{ 0 };
		float closeRate{ 0.0f };
		unsigned hullRoles{ 0 };
		HullSize hullSize{ HullSize::Small };
		GoodAmount steelCost{ 0.f };
		long long buildTicks{ 0 };
		long long missileTubes{ 0 };
		long long missileReloadInnerTicks{ 0 };
		long long magazineSize{ 0 };
		long long missileDamage{ 0 };
		float missileRange{ 0.0f };
		float missileSpeed{ 0.0f };
		long long reactorCount{ 0 };
		long long engineCount{ 0 };
		long long laserCount{ 0 };
		long long missileTubeCount{ 0 };
		long long cargoCount{ 0 };
	};

	struct ClassCount
	{
		long long classId{ 0 };
		long long count{ 0 };
	};

	struct FleetHull
	{
		long long id{ 0 };
		long long classId{ 0 };
		std::vector<GoodQuantity> magazine;
	};

	struct ClassDesiredCargo
	{
		long long classId{ 0 };
		DesiredCargo desired;
	};

	struct Fleet
	{
		long long id{ 0 };
		long long empireId{ 0 };
		std::string name;
		long long systemId{ 0 };
		std::vector<ClassCount> composition;
		std::vector<FleetHull> hulls;
		LaneMotion motion;
		long long policyId{ 0 };
		long long escortConvoyId{ 0 };
		std::vector<ClassDesiredCargo> magazineCargoByClass;
	};

	struct BattleSideReport
	{
		long long fleetId{ 0 };
		long long empireId{ 0 };
		std::vector<ClassCount> before;
		std::vector<ClassCount> after;
		long long missilesBefore{ 0 };
		long long missilesAfter{ 0 };
	};

	struct BattleReport
	{
		long long id{ 0 };
		long long tick{ 0 };
		long long systemId{ 0 };
		std::vector<BattleSideReport> sides;
	};

	struct CivilianLoss
	{
		long long systemId{ 0 };
		long long tick{ 0 };
		long long victimEmpireId{ 0 };
	};

	struct HaulJob
	{
		long long id{ 0 };
		long long destinationSystemId{ 0 };
		long long empireId{ 0 };
		long long goodId{ 0 };
		GoodAmount amount{ 0.f };
	};

	struct Convoy
	{
		long long id{ 0 };
		long long haulJobId{ 0 };
		long long empireId{ 0 };
		long long loadSystemId{ 0 };
		LaneMotion motion;
		long long escortFleetId{ 0 };
		long long desiredCivilianTruckCount{ 0 };
		long long desiredMilitaryTruckCount{ 0 };
		long long desiredTenderCount{ 0 };
		long long expectedArrivalTick{ 0 };
		DesiredCargo holdCargo;
	};

	struct WorldData
	{
		long long tick{ 0 };
		std::uint64_t masterSeed{ 1 };
		std::vector<Empire> empires;
		std::vector<StarSystem> systems;
		std::vector<Hyperlane> hyperlanes;
		std::vector<Mine> mines;
		std::vector<Yard> yards;
		std::vector<Factory> factories;
		std::vector<Truck> trucks;
		std::vector<HaulJob> haulJobs;
		std::vector<Convoy> convoys;
		std::vector<ShipClass> shipClasses;
		std::vector<Fleet> fleets;
		std::vector<EngagementPolicy> engagementPolicies;
		std::vector<BattleReport> battleReports;
		std::vector<CivilianLoss> civilianLosses;
		std::vector<EmpireWar> empireWars;
		std::vector<CivilianHaulForbid> civilianHaulForbids;
		std::vector<ResearchProgress> researchProgress;

		const Empire* empireById(long long id) const;
		Empire* empireById(long long id);
		const StarSystem* systemById(long long id) const;
		StarSystem* systemById(long long id);
		const ShipClass* shipClassById(long long id) const;
		ShipClass* shipClassById(long long id);
		const Fleet* fleetById(long long id) const;
		Fleet* fleetById(long long id);
		const HaulJob* haulJobById(long long id) const;
		HaulJob* haulJobById(long long id);
		const Convoy* convoyById(long long id) const;
		Convoy* convoyById(long long id);
		const Truck* truckById(long long id) const;
		Truck* truckById(long long id);
		const EngagementPolicy* engagementPolicyById(long long id) const;
		EngagementPolicy* engagementPolicyById(long long id);
		const ResearchProgress* researchProgressByEmpireAndProject(
			long long empireId,
			long long projectId) const;
		ResearchProgress* researchProgressByEmpireAndProject(long long empireId, long long projectId);
	};

	bool shipClassHasRole(const ShipClass& shipClass, HullRole role);
	bool truckHasRole(const WorldData& world, const Truck& truck, HullRole role);
	bool truckHasCargoHaulRole(const WorldData& world, const Truck& truck);
	bool truckUsesMilitaryHaulPath(const WorldData& world, const Truck& truck);
	bool ruleOfEngagementOpensFire(RuleOfEngagement rule);
	const char* ruleOfEngagementName(RuleOfEngagement rule);
	const char* fleetApproachName(FleetApproach approach);
	const EngagementPolicy* activeEngagementPolicy(const WorldData& world, const Fleet& fleet);
	bool anyFleetOpensFire(const WorldData& world, const std::vector<long long>& fleetIds);

	GoodAmount getStockpileAmount(const StarSystem& system, long long goodId);
	GoodAmount locatedGoodAmount(const WorldData& world, long long goodId);
	GoodAmount getInbound(const StarSystem& system, long long goodId);
	GoodAmount getNeed(const StarSystem& system, long long goodId);
	GoodAmount uncoveredNeed(const StarSystem& system, long long goodId);
	GoodAmount quantityForGood(const std::vector<GoodQuantity>& rows, long long goodId);
	GoodAmount mineOutputPerTick(const Mine& mine, long long goodId);
	GoodAmount mineMaxStockpile(const Mine& mine, long long goodId);
	const ProductRampSpec* productRampSpecForGood(long long goodId);
	const FactoryRecipe* factoryRecipeById(long long recipeId);
	Factory makeFactoryFromRecipe(
		long long systemId,
		long long empireId,
		const FactoryRecipe& recipe,
		std::vector<GoodQuantity> desiredPiles = {});
	void applyFactoryRecipe(Factory& factory, const FactoryRecipe& recipe);
	void retargetFactoryRecipe(Factory& factory, const FactoryRecipe& recipe);
	GoodAmount factoryEffectiveAmount(GoodAmount ratedAmount, long long efficiencyPerThousand);
	void advanceProductRampAfterProduction(
		Factory& factory,
		const ProductRampSpec& spec,
		GoodAmount outputAdded);
	GoodAmount consumerDesiredPile(const WorldData& world, long long systemId, long long goodId);
	GoodAmount stockpileAvailableForYardConsume(const WorldData& world, long long systemId, long long goodId);
	void postNeedFromConsumerShortfall(WorldData& world, StarSystem& system, long long goodId);
	void postNeedsFromConsumerShortfalls(WorldData& world, StarSystem& system);
	bool useFromStockpile(WorldData& world, long long systemId, long long goodId, GoodAmount amount);
	void applyDeferredNeedPosts(WorldData& world);
	void syncHaulJobsFromUncoveredNeed(WorldData& world);
	const HaulJob* haulJobFor(const WorldData& world, long long destinationSystemId, long long goodId);
	long long truckSystemId(const Truck& truck);
	long long nearestSupplySystemId(
		const WorldData& world,
		long long fromSystemId,
		long long goodId,
		long long destinationSystemId,
		long long viewerEmpireId,
		bool seesEscort = false,
		bool isMilitary = false);
	Convoy* openConvoyForHaulJob(WorldData& world, long long haulJobId, bool isMilitary);
	void parkTruckAt(Truck& truck, long long systemId);
	long long truckCountToCarryAmountInOneGo(GoodAmount amount, long long goodId);
	bool convoyCarriesAmountInOneGo(long long truckCount, GoodAmount amount, long long goodId);
	bool convoyIsUnderway(const Convoy& convoy);
	bool convoyIsStanding(const Convoy& convoy);
	bool convoyJobGroupIsMilitary(const WorldData& world, const Convoy& convoy);
	bool standingConvoyUsesMilitaryHaulPath(const Convoy& convoy);
	long long standingDesiredTruckCount(const Convoy& convoy);
	long long countMilitaryTrucksInConvoy(const WorldData& world, long long convoyId);
	long long countCivilianTrucksInConvoy(const WorldData& world, long long convoyId);
	long long countTendersInConvoy(const WorldData& world, long long convoyId);
	long long countCargoHaulTrucksInConvoy(const WorldData& world, long long convoyId);
	bool standingConvoyHasJoinSlotForTruck(const WorldData& world, const Convoy& convoy, const Truck& truck);
	long long nextConvoyId(const WorldData& world);
	long long fleetRallySystemId(const Fleet& fleet);
	long long standingRallySystemId(const WorldData& world, const Convoy& convoy);
	bool pathTruckToSystem(WorldData& world, Truck& truck, long long systemId);
	long long countTrucksInConvoy(const WorldData& world, long long convoyId);
	long long countTrucksInConvoyPresent(const WorldData& world, const Convoy& convoy);
	long long emptyTrucksInConvoy(const WorldData& world, long long convoyId);
	bool haulWaveHasStockpileForAnotherTruck(
		const WorldData& world,
		long long loadSystemId,
		long long goodId,
		long long assemblingConvoyId);
	GoodAmount convoyCargoOrReservedAmount(const WorldData& world, const Convoy& convoy);
	void removeConvoyIfNoMembers(WorldData& world, long long convoyId);
	bool isTruckAtSystem(const Truck& truck, long long systemId);
	long long countTrucksAtSystem(const WorldData& world, long long systemId);
	bool hasHyperlane(const WorldData& world, long long a, long long b);
	long long fleetShipCount(const Fleet& fleet);
	bool fleetIsParked(const Fleet& fleet);
	long long classCountFor(const std::vector<ClassCount>& composition, long long classId);
	long long nextFleetHullId(const WorldData& world);
	long long fleetMissileAmmo(const Fleet& fleet);
	bool fleetHasJoinedMissileAmmo(const Fleet& fleet);
	long long fleetMissileAmmoForClass(const Fleet& fleet, long long classId);
	long long fleetMagazineMaxForClass(const WorldData& world, const Fleet& fleet, long long classId);
	long long fleetWarshipMagazineCapacity(const WorldData& world, const Fleet& fleet);
	DesiredCargo desiredCargoForFleetClass(const Fleet& fleet, long long classId);
	void ensureFleetClassDesiredCargo(Fleet& fleet, long long classId);
	void setFleetClassDesiredCargo(Fleet& fleet, long long classId, DesiredCargo desired);
	std::vector<GoodQuantity> truckCargoAsQuantities(const Truck& truck);
	bool hullWantsCombatMissileGood(
		const DesiredCargo& magazineCargo,
		const FleetHull& hull,
		GoodAmount magazineSize,
		long long goodId);
	void syncFleetCompositionFromHulls(Fleet& fleet);
	bool removeFleetHull(Fleet& fleet, long long hullId);
	long long nextFleetId(const WorldData& world);
	void appendFullMagazineHulls(WorldData& world, Fleet& fleet, long long classId, long long count);
	void appendEmptyMagazineHulls(WorldData& world, Fleet& fleet, long long classId, long long count);
	long long systemOwnershipScore(const WorldData& world, long long systemId);
	SystemOwnershipType systemOwnershipType(const WorldData& world, long long systemId);
	const char* systemOwnershipTypeName(SystemOwnershipType type);
	long long systemPresenceScore(const WorldData& world, long long systemId);
	bool systemHasParkedHostileFleet(const WorldData& world, long long systemId, long long viewerEmpireId);
	bool systemIsAdjacentToParkedHostileFleet(
		const WorldData& world,
		long long systemId,
		long long viewerEmpireId);
	bool hazardEventIsHot(long long eventTick, long long worldTick);
	bool systemHasHazardHeat(const WorldData& world, long long systemId, long long viewerEmpireId);
	bool systemIsAdjacentToHazardHeat(
		const WorldData& world,
		long long systemId,
		long long viewerEmpireId);
	bool empiresAreAtWar(const WorldData& world, long long empireA, long long empireB);
	void declareWarBetweenEmpires(WorldData& world, long long empireA, long long empireB);
	bool systemClaimBlocksCivilian(
		const WorldData& world,
		long long systemId,
		long long viewerEmpireId);
	bool civilianHaulIsForbidden(
		const WorldData& world,
		long long empireId,
		long long forbiddenEmpireId);
	void setCivilianHaulForbidden(
		WorldData& world,
		long long empireId,
		long long forbiddenEmpireId,
		bool forbidden);
	const Yard* yardAtSystem(const WorldData& world, long long systemId);
	Yard* yardAtSystem(WorldData& world, long long systemId);
	long long nextYardSlipId(const WorldData& world);
	long long countIdleYardSlips(const Yard& yard);
	void applyYardSlipConstruction(WorldData& world);
	bool shipClassIsTruckOfEmpire(const ShipClass& shipClass, long long empireId);
	bool shipClassIsWarshipOfEmpire(const ShipClass& shipClass, long long empireId);
	GoodAmount hullSteelPerTick(const ShipClass& shipClass);
	GoodAmount refitSteelPerTick();
	long long nextTruckId(const WorldData& world);
	void applyYardHullConstruction(WorldData& world);
	void applyYardMagazineRestock(WorldData& world);
	void applyTenderLoadFromYard(WorldData& world);
	void applyTenderUnloadIntoMagazines(WorldData& world);

}
