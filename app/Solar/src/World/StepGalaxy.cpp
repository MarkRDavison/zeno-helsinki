#include <World/StepGalaxy.hpp>
#include <World/Goods.hpp>
#include <algorithm>
#include <utility>

namespace slr
{
namespace
{

	void addToStockpile(StarSystem& system, long long goodId, long long amount)
	{
		for (auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				stock.goodAmount += amount;
				return;
			}
		}

		system.stocks.push_back(GoodStock{ .goodId = goodId, .goodAmount = amount });
	}

	bool takeFromStockpile(StarSystem& system, long long goodId, long long amount)
	{
		for (auto& stock : system.stocks)
		{
			if (stock.goodId == goodId && stock.goodAmount >= amount)
			{
				stock.goodAmount -= amount;
				return true;
			}
		}

		return false;
	}

	void reduceNeed(StarSystem& system, long long goodId, long long amount)
	{
		for (auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				stock.need = std::max(0LL, stock.need - amount);
				return;
			}
		}
	}

	void applyExtraction(WorldData& world)
	{
		for (const auto& mine : world.mines)
		{
			StarSystem* system = world.systemById(mine.systemId);
			if (system == nullptr)
			{
				continue;
			}

			addToStockpile(*system, mine.goodId, mine.outputPerTick);
		}
	}

	bool hasHyperlane(const WorldData& world, long long a, long long b)
	{
		if (a > b)
		{
			std::swap(a, b);
		}

		for (const auto& lane : world.hyperlanes)
		{
			if (lane.a == a && lane.b == b)
			{
				return true;
			}
		}

		return false;
	}

	bool hopEndpoints(const Truck& truck, long long& fromId, long long& toId)
	{
		if (truck.route.size() < 2)
		{
			return false;
		}

		const int toIndex = truck.hop + truck.direction;
		if (truck.hop < 0 || toIndex < 0)
		{
			return false;
		}

		if (truck.hop >= static_cast<int>(truck.route.size()) ||
			toIndex >= static_cast<int>(truck.route.size()))
		{
			return false;
		}

		fromId = truck.route[static_cast<size_t>(truck.hop)];
		toId = truck.route[static_cast<size_t>(toIndex)];
		return true;
	}

	void loadIfDemandOpen(WorldData& world, Truck& truck)
	{
		if (truck.route.size() < 2 || truck.cargoAmount != 0)
		{
			return;
		}

		const long long supplyId = truck.route.front();
		const long long demandId = truck.route.back();
		if (!isTruckAtSystem(truck, supplyId))
		{
			return;
		}

		StarSystem* demand = world.systemById(demandId);
		StarSystem* supply = world.systemById(supplyId);
		if (demand == nullptr || supply == nullptr)
		{
			return;
		}

		if (getNeed(*demand, SteelGoodId) <= 0)
		{
			return;
		}

		if (!takeFromStockpile(*supply, SteelGoodId, TruckCapacity_Steel))
		{
			return;
		}

		truck.cargoGoodId = SteelGoodId;
		truck.cargoAmount = TruckCapacity_Steel;
	}

	void dumpIfCarrying(WorldData& world, Truck& truck)
	{
		if (truck.route.size() < 2 || truck.cargoAmount <= 0)
		{
			return;
		}

		const long long demandId = truck.route.back();
		if (!isTruckAtSystem(truck, demandId))
		{
			return;
		}

		StarSystem* demand = world.systemById(demandId);
		if (demand == nullptr)
		{
			return;
		}

		addToStockpile(*demand, truck.cargoGoodId, truck.cargoAmount);
		reduceNeed(*demand, truck.cargoGoodId, truck.cargoAmount);
		truck.cargoGoodId = 0;
		truck.cargoAmount = 0;
	}

	bool waitEmptyAtSupply(const Truck& truck)
	{
		if (truck.route.size() < 2 || truck.cargoAmount != 0)
		{
			return false;
		}

		return isTruckAtSystem(truck, truck.route.front());
	}

	void advanceTruck(WorldData& world, Truck& truck)
	{
		long long fromId = 0;
		long long toId = 0;
		if (!hopEndpoints(truck, fromId, toId))
		{
			return;
		}

		const StarSystem* from = world.systemById(fromId);
		const StarSystem* to = world.systemById(toId);
		if (from == nullptr || to == nullptr)
		{
			return;
		}

		if (!hasHyperlane(world, fromId, toId))
		{
			return;
		}

		const float length = glm::length(to->position - from->position);
		if (length <= 0.0f || truck.speed <= 0.0f)
		{
			return;
		}

		truck.progress = std::min(1.0f, truck.progress + truck.speed / length);
		if (truck.progress < 1.0f)
		{
			return;
		}

		truck.hop += truck.direction;
		truck.progress = 0.0f;
		if (truck.hop == 0 || truck.hop == static_cast<int>(truck.route.size()) - 1)
		{
			truck.direction = -truck.direction;
		}
	}

	void moveTrucks(WorldData& world)
	{
		for (auto& truck : world.trucks)
		{
			loadIfDemandOpen(world, truck);
			dumpIfCarrying(world, truck);
			if (waitEmptyAtSupply(truck))
			{
				continue;
			}

			advanceTruck(world, truck);
		}
	}

}

	void stepGalaxy(WorldData& world)
	{
		applyExtraction(world);
		moveTrucks(world);
		++world.tick;
	}

}
