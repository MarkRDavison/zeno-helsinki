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

	struct Truck
	{
		long long id{ 0 };
		std::vector<long long> route;
		int hop{ 0 };
		float progress{ 0.0f };
		int direction{ 1 };
		float speed{ 0.0f };
		long long cargoGoodId{ 0 };
		long long cargoAmount{ 0 };
	};

	struct WorldData
	{
		long long tick{ 0 };
		std::vector<StarSystem> systems;
		std::vector<Hyperlane> hyperlanes;
		std::vector<Mine> mines;
		std::vector<Truck> trucks;

		const StarSystem* systemById(long long id) const;
		StarSystem* systemById(long long id);
	};

	long long getStockpileAmount(const StarSystem& system, long long goodId);
	long long getNeed(const StarSystem& system, long long goodId);
	bool isTruckAtSystem(const Truck& truck, long long systemId);
	long long countTrucksAtSystem(const WorldData& world, long long systemId);

}
