#pragma once

#include <helsinki/System/glm.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace slr
{

	constexpr long long HelionEmpireId = 1;
	constexpr long long AshreachEmpireId = 2;

	struct Empire
	{
		long long id{ 0 };
		std::string name;
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

	struct Yard
	{
		long long systemId{ 0 };
		std::vector<GoodQuantity> desiredPiles;
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
		LaneMotion motion;
		long long cargoGoodId{ 0 };
		long long cargoAmount{ 0 };
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
	};

	struct ClassCount
	{
		long long classId{ 0 };
		long long count{ 0 };
	};

	struct Fleet
	{
		long long id{ 0 };
		long long empireId{ 0 };
		std::string name;
		long long systemId{ 0 };
		std::vector<ClassCount> composition;
		LaneMotion motion;
	};

	struct BattleSideReport
	{
		long long fleetId{ 0 };
		long long empireId{ 0 };
		std::vector<ClassCount> before;
		std::vector<ClassCount> after;
	};

	struct BattleReport
	{
		long long id{ 0 };
		long long tick{ 0 };
		long long systemId{ 0 };
		std::vector<BattleSideReport> sides;
	};

	struct HaulJob
	{
		long long id{ 0 };
		long long destinationSystemId{ 0 };
		long long goodId{ 0 };
		long long amount{ 0 };
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
		std::vector<Truck> trucks;
		std::vector<HaulJob> haulJobs;
		std::vector<ShipClass> shipClasses;
		std::vector<Fleet> fleets;
		std::vector<BattleReport> battleReports;

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
	};

	long long getStockpileAmount(const StarSystem& system, long long goodId);
	long long getInbound(const StarSystem& system, long long goodId);
	long long getNeed(const StarSystem& system, long long goodId);
	long long uncoveredNeed(const StarSystem& system, long long goodId);
	long long quantityForGood(const std::vector<GoodQuantity>& rows, long long goodId);
	long long mineOutputPerTick(const Mine& mine, long long goodId);
	long long mineMaxStockpile(const Mine& mine, long long goodId);
	long long consumerDesiredPile(const WorldData& world, long long systemId, long long goodId);
	void postNeedFromConsumerShortfall(WorldData& world, StarSystem& system, long long goodId);
	void postNeedsFromConsumerShortfalls(WorldData& world, StarSystem& system);
	bool useFromStockpile(WorldData& world, long long systemId, long long goodId, long long amount);
	void applyDeferredNeedPosts(WorldData& world);
	void syncHaulJobsFromUncoveredNeed(WorldData& world);
	const HaulJob* haulJobFor(const WorldData& world, long long destinationSystemId, long long goodId);
	long long truckCountToCarryAmountInOneGo(long long amount, long long goodId);
	bool convoyCarriesAmountInOneGo(long long truckCount, long long amount, long long goodId);
	bool isTruckAtSystem(const Truck& truck, long long systemId);
	long long countTrucksAtSystem(const WorldData& world, long long systemId);
	bool hasHyperlane(const WorldData& world, long long a, long long b);
	long long fleetShipCount(const Fleet& fleet);
	long long classCountFor(const std::vector<ClassCount>& composition, long long classId);

}
