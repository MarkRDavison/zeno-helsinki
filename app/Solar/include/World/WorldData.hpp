#pragma once

#include <helsinki/System/glm.hpp>
#include <string>
#include <vector>

namespace slr
{

	struct GoodStock
	{
		long long goodId{ 0 };
		long long goodAmount{ 0 };
		long long need{ 0 };
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

	struct Mine
	{
		long long systemId{ 0 };
		long long goodId{ 0 };
		long long outputPerTick{ 0 };
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
		std::string name;
	};

	struct ClassCount
	{
		long long classId{ 0 };
		long long count{ 0 };
	};

	struct Fleet
	{
		long long id{ 0 };
		std::string name;
		long long systemId{ 0 };
		std::vector<ClassCount> composition;
		LaneMotion motion;
	};

	struct WorldData
	{
		long long tick{ 0 };
		std::vector<StarSystem> systems;
		std::vector<Hyperlane> hyperlanes;
		std::vector<Mine> mines;
		std::vector<Truck> trucks;
		std::vector<ShipClass> shipClasses;
		std::vector<Fleet> fleets;

		const StarSystem* systemById(long long id) const;
		StarSystem* systemById(long long id);
		const ShipClass* shipClassById(long long id) const;
		ShipClass* shipClassById(long long id);
		const Fleet* fleetById(long long id) const;
		Fleet* fleetById(long long id);
	};

	long long getStockpileAmount(const StarSystem& system, long long goodId);
	long long getNeed(const StarSystem& system, long long goodId);
	bool isTruckAtSystem(const Truck& truck, long long systemId);
	long long countTrucksAtSystem(const WorldData& world, long long systemId);
	bool hasHyperlane(const WorldData& world, long long a, long long b);

}
