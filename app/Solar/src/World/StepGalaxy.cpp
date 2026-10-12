#include <World/StepGalaxy.hpp>
#include <World/Combat.hpp>
#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <World/TruckCargo.hpp>
#include <algorithm>
#include <vector>

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

	void setRouteToDestination(WorldData& world, Truck& truck, long long fromId, long long destId)
	{
		const std::vector<long long> path = shortestLanePath(world, fromId, destId);
		if (path.size() < 2)
		{
			parkTruckAt(truck, fromId);
			return;
		}

		truck.motion.route = path;
		truck.motion.hop = 0;
		truck.motion.progress = 0.0f;
		truck.motion.direction = 1;
	}

	void loadIfJoinedJob(WorldData& world, Truck& truck)
	{
		if (truck.haulJobId == 0 || truck.cargoAmount != 0)
		{
			return;
		}

		HaulJob* job = world.haulJobById(truck.haulJobId);
		if (job == nullptr)
		{
			return;
		}

		const long long here = truckSystemId(truck);
		if (here == 0)
		{
			return;
		}

		const long long supplyId = nearestSupplySystemId(
			world,
			here,
			job->goodId,
			job->destinationSystemId);
		if (here != supplyId)
		{
			return;
		}

		StarSystem* supply = world.systemById(here);
		StarSystem* demand = world.systemById(job->destinationSystemId);
		if (supply == nullptr || demand == nullptr)
		{
			return;
		}

		const long long truckload = truckloadForGood(job->goodId);
		if (!takeFromStockpile(*supply, job->goodId, truckload))
		{
			return;
		}

		truck.cargoGoodId = job->goodId;
		truck.cargoAmount = truckload;
		raiseInboundOnLoad(*demand, job->goodId, truckload);
		if (here != job->destinationSystemId)
		{
			setRouteToDestination(world, truck, here, job->destinationSystemId);
		}
	}

	void dumpIfCarrying(WorldData& world, Truck& truck)
	{
		if (truck.cargoAmount <= 0 || truck.haulJobId == 0)
		{
			return;
		}

		HaulJob* job = world.haulJobById(truck.haulJobId);
		if (job == nullptr)
		{
			return;
		}

		if (!isTruckAtSystem(truck, job->destinationSystemId))
		{
			return;
		}

		StarSystem* demand = world.systemById(job->destinationSystemId);
		if (demand == nullptr)
		{
			return;
		}

		reduceInboundOnArrival(*demand, truck.cargoGoodId, truck.cargoAmount);
		addToStockpile(*demand, truck.cargoGoodId, truck.cargoAmount);
		reduceNeed(*demand, truck.cargoGoodId, truck.cargoAmount);
		truck.cargoGoodId = 0;
		truck.cargoAmount = 0;
		truck.haulJobId = 0;
		parkTruckAt(truck, job->destinationSystemId);
	}

	void advanceTruck(WorldData& world, Truck& truck)
	{
		advanceAlongLane(world, truck.motion);
	}

	void moveTrucks(WorldData& world)
	{
		for (auto& truck : world.trucks)
		{
			loadIfJoinedJob(world, truck);
			dumpIfCarrying(world, truck);
			advanceTruck(world, truck);
		}
	}

	void applyHaulJobAccepts(WorldData& world)
	{
		std::vector<GameCommand> accepts;
		for (const auto& truck : world.trucks)
		{
			if (truck.cargoAmount != 0 || truck.haulJobId != 0)
			{
				continue;
			}

			long long bestJobId = 0;
			for (const auto& job : world.haulJobs)
			{
				if (job.empireId != truck.empireId)
				{
					continue;
				}

				if (job.amount < truckloadForGood(job.goodId))
				{
					continue;
				}

				if (bestJobId == 0 || job.id < bestJobId)
				{
					bestJobId = job.id;
				}
			}

			if (bestJobId != 0)
			{
				accepts.push_back(GameCommand::acceptHaulJob(truck.id, bestJobId, CommandSource::Ai));
			}
		}

		std::sort(
			accepts.begin(),
			accepts.end(),
			[](const GameCommand& a, const GameCommand& b)
			{
				return std::get<AcceptHaulJob>(a.payload).truckId <
					std::get<AcceptHaulJob>(b.payload).truckId;
			});

		for (const auto& command : accepts)
		{
			execute(world, command);
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
		applyDeferredNeedPosts(world);
		syncHaulJobsFromUncoveredNeed(world);
		applyExtraction(world);
		applyHaulJobAccepts(world);
		moveTrucks(world);
		syncHaulJobsFromUncoveredNeed(world);
		moveFleets(world);
		resolveBattles(world);
		++world.tick;
	}

}
