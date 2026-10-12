#pragma once

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

	enum class RuleOfEngagement
	{
		DoNotFireUnlessFiredUpon,
		FireIfEnemySeen
	};

	enum class SystemOwnershipType
	{
		Empty,
		Extraction,
		Industrial,
		Populated
	};

	struct EngagementPolicy
	{
		long long id{ 0 };
		long long empireId{ 0 };
		std::string name;
		RuleOfEngagement ruleOfEngagement{ RuleOfEngagement::DoNotFireUnlessFiredUpon };
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
		long long goodAmount{ 0 };
		long long inbound{ 0 };
		long long need{ 0 };
		long long deferredNeed{ 0 };
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

	struct GoodQuantity
	{
		long long goodId{ 0 };
		long long amount{ 0 };
	};

	struct MineOutput
	{
		long long goodId{ 0 };
		long long amountPerTick{ 0 };
		long long maxStockpile{ 0 };
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
	};

	struct Factory
	{
		long long systemId{ 0 };
		long long empireId{ 0 };
		std::vector<GoodQuantity> desiredPiles;
		long long inputGoodId{ 0 };
		long long inputAmountPerTick{ 0 };
		long long outputGoodId{ 0 };
		long long outputAmountPerTick{ 0 };
		long long outputMaxStockpile{ 0 };
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
		long long cargoAmount{ 0 }; 
		long long haulJobId{ 0 };
		long long convoyId{ 0 };
		long long classId{ 0 };
		bool isMilitary{ false };
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
		bool isTruck{ false };
		bool isMilitary{ false };
		long long steelCost{ 0 };
		long long buildTicks{ 0 };
		long long missileTubes{ 0 };
		long long missileReloadInnerTicks{ 0 };
		long long magazineSize{ 0 };
		long long missileDamage{ 0 };
		float missileRange{ 0.0f };
		float missileSpeed{ 0.0f };
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
		long long missileAmmo{ 0 };
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
		long long amount{ 0 };
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
		long long expectedArrivalTick{ 0 };
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
	};

	bool ruleOfEngagementOpensFire(RuleOfEngagement rule);
	const char* ruleOfEngagementName(RuleOfEngagement rule);
	const EngagementPolicy* activeEngagementPolicy(const WorldData& world, const Fleet& fleet);
	bool anyFleetOpensFire(const WorldData& world, const std::vector<long long>& fleetIds);

	long long getStockpileAmount(const StarSystem& system, long long goodId);
	long long locatedGoodAmount(const WorldData& world, long long goodId);
	long long getInbound(const StarSystem& system, long long goodId);
	long long getNeed(const StarSystem& system, long long goodId);
	long long uncoveredNeed(const StarSystem& system, long long goodId);
	long long quantityForGood(const std::vector<GoodQuantity>& rows, long long goodId);
	long long mineOutputPerTick(const Mine& mine, long long goodId);
	long long mineMaxStockpile(const Mine& mine, long long goodId);
	long long consumerDesiredPile(const WorldData& world, long long systemId, long long goodId);
	long long stockpileAvailableForYardConsume(const WorldData& world, long long systemId, long long goodId);
	void postNeedFromConsumerShortfall(WorldData& world, StarSystem& system, long long goodId);
	void postNeedsFromConsumerShortfalls(WorldData& world, StarSystem& system);
	bool useFromStockpile(WorldData& world, long long systemId, long long goodId, long long amount);
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
	long long truckCountToCarryAmountInOneGo(long long amount, long long goodId);
	bool convoyCarriesAmountInOneGo(long long truckCount, long long amount, long long goodId);
	bool convoyIsUnderway(const Convoy& convoy);
	bool convoyIsStanding(const Convoy& convoy);
	bool convoyJobGroupIsMilitary(const WorldData& world, const Convoy& convoy);
	bool standingConvoyUsesMilitaryHaulPath(const Convoy& convoy);
	long long standingDesiredTruckCount(const Convoy& convoy);
	long long countMilitaryTrucksInConvoy(const WorldData& world, long long convoyId);
	long long countCivilianTrucksInConvoy(const WorldData& world, long long convoyId);
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
	long long convoyCargoOrReservedAmount(const WorldData& world, const Convoy& convoy);
	void removeConvoyIfNoMembers(WorldData& world, long long convoyId);
	bool isTruckAtSystem(const Truck& truck, long long systemId);
	long long countTrucksAtSystem(const WorldData& world, long long systemId);
	bool hasHyperlane(const WorldData& world, long long a, long long b);
	long long fleetShipCount(const Fleet& fleet);
	bool fleetIsParked(const Fleet& fleet);
	long long classCountFor(const std::vector<ClassCount>& composition, long long classId);
	long long nextFleetHullId(const WorldData& world);
	long long fleetMissileAmmo(const Fleet& fleet);
	long long fleetMissileAmmoForClass(const Fleet& fleet, long long classId);
	long long fleetMagazineMaxForClass(const WorldData& world, const Fleet& fleet, long long classId);
	void syncFleetCompositionFromHulls(Fleet& fleet);
	void appendFullMagazineHulls(WorldData& world, Fleet& fleet, long long classId, long long count);
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
	long long hullSteelPerTick(const ShipClass& shipClass);
	long long nextTruckId(const WorldData& world);
	void applyYardHullConstruction(WorldData& world);

}
