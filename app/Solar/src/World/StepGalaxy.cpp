#include <World/StepGalaxy.hpp>
#include <World/Combat.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <World/TruckCargo.hpp>
#include <algorithm>

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

			for (const auto& output : mine.outputsPerTick)
			{
				const long long room = output.maxStockpile - getStockpileAmount(*system, output.goodId);
				if (room <= 0 || output.amountPerTick <= 0)
				{
					continue;
				}

				addToStockpile(*system, output.goodId, std::min(output.amountPerTick, room));
			}
		}
	}

	void loadIfDemandOpen(WorldData& world, Truck& truck)
	{
		if (truck.motion.route.size() < 2 || truck.cargoAmount != 0)
		{
			return;
		}

		const long long supplyId = truck.motion.route.front();
		const long long demandId = truck.motion.route.back();
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

		if (uncoveredNeed(*demand, SteelGoodId) < TruckCapacity_Steel)
		{
			return;
		}

		if (!takeFromStockpile(*supply, SteelGoodId, TruckCapacity_Steel))
		{
			return;
		}

		truck.cargoGoodId = SteelGoodId;
		truck.cargoAmount = TruckCapacity_Steel;
		raiseInboundOnLoad(*demand, SteelGoodId, TruckCapacity_Steel);
	}

	void dumpIfCarrying(WorldData& world, Truck& truck)
	{
		if (truck.motion.route.size() < 2 || truck.cargoAmount <= 0)
		{
			return;
		}

		const long long demandId = truck.motion.route.back();
		if (!isTruckAtSystem(truck, demandId))
		{
			return;
		}

		StarSystem* demand = world.systemById(demandId);
		if (demand == nullptr)
		{
			return;
		}

		reduceInboundOnArrival(*demand, truck.cargoGoodId, truck.cargoAmount);
		addToStockpile(*demand, truck.cargoGoodId, truck.cargoAmount);
		reduceNeed(*demand, truck.cargoGoodId, truck.cargoAmount);
		truck.cargoGoodId = 0;
		truck.cargoAmount = 0;
	}

	bool waitEmptyAtSupply(const Truck& truck)
	{
		if (truck.motion.route.size() < 2 || truck.cargoAmount != 0)
		{
			return false;
		}

		return isTruckAtSystem(truck, truck.motion.route.front());
	}

	void advanceTruck(WorldData& world, Truck& truck)
	{
		if (!advanceAlongLane(world, truck.motion))
		{
			return;
		}

		if (truck.motion.hop == 0 ||
			truck.motion.hop == static_cast<int>(truck.motion.route.size()) - 1)
		{
			truck.motion.direction = -truck.motion.direction;
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

	void advanceFleet(WorldData& world, Fleet& fleet)
	{
		if (!advanceAlongLane(world, fleet.motion))
		{
			return;
		}

		if (fleet.motion.hop < 0 ||
			fleet.motion.hop >= static_cast<int>(fleet.motion.route.size()))
		{
			return;
		}

		fleet.systemId = fleet.motion.route[static_cast<size_t>(fleet.motion.hop)];
		if (fleet.motion.hop == static_cast<int>(fleet.motion.route.size()) - 1)
		{
			fleet.motion.route.clear();
			fleet.motion.hop = 0;
		}
	}

	void moveFleets(WorldData& world)
	{
		for (auto& fleet : world.fleets)
		{
			advanceFleet(world, fleet);
		}
	}

}

	void stepGalaxy(WorldData& world)
	{
		applyExtraction(world);
		moveTrucks(world);
		moveFleets(world);
		resolveBattles(world);
		++world.tick;
	}

}
