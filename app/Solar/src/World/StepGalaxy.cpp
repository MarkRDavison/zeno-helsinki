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

	void applyFactoryProduction(WorldData& world)
	{
		for (const auto& factory : world.factories)
		{
			StarSystem* system = world.systemById(factory.systemId);
			if (system == nullptr)
			{
				continue;
			}

			if (factory.inputAmountPerTick <= 0 || factory.outputAmountPerTick <= 0)
			{
				continue;
			}

			const long long outputRoom =
				factory.outputMaxStockpile - getStockpileAmount(*system, factory.outputGoodId);
			if (outputRoom < factory.outputAmountPerTick)
			{
				continue;
			}

			if (!useFromStockpile(world, factory.systemId, factory.inputGoodId, factory.inputAmountPerTick))
			{
				continue;
			}

			addToStockpile(*system, factory.outputGoodId, factory.outputAmountPerTick);
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

	void copyConvoyMotionOntoMembers(WorldData& world, const Convoy& convoy)
	{
		for (auto& truck : world.trucks)
		{
			if (truck.convoyId != convoy.id)
			{
				continue;
			}

			const float speed = truck.motion.speed;
			truck.motion.route = convoy.motion.route;
			truck.motion.hop = convoy.motion.hop;
			truck.motion.progress = convoy.motion.progress;
			truck.motion.direction = convoy.motion.direction;
			truck.motion.speed = speed;
		}
	}

	bool convoyMembersReadyToSail(const WorldData& world, const Convoy& convoy)
	{
		bool anyLoadedAtRally = false;
		bool anyMember = false;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId != convoy.id)
			{
				continue;
			}

			anyMember = true;
			if (!isTruckAtSystem(truck, convoy.loadSystemId))
			{
				return false;
			}

			if (truck.cargoAmount > 0)
			{
				anyLoadedAtRally = true;
			}
		}

		return anyMember && anyLoadedAtRally;
	}

	float slowestMemberSpeed(const WorldData& world, long long convoyId)
	{
		float slowest = 0.0f;
		bool any = false;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId != convoyId)
			{
				continue;
			}

			if (!any || truck.motion.speed < slowest)
			{
				slowest = truck.motion.speed;
				any = true;
			}
		}

		return slowest;
	}

	long long nextConvoyId(const WorldData& world)
	{
		long long nextId = 1;
		for (const auto& convoy : world.convoys)
		{
			if (convoy.id >= nextId)
			{
				nextId = convoy.id + 1;
			}
		}

		return nextId;
	}

	void departReadyConvoys(WorldData& world)
	{
		struct LeftoverWave
		{
			long long haulJobId{ 0 };
			long long empireId{ 0 };
			long long loadSystemId{ 0 };
			std::vector<long long> truckIds;
		};

		std::vector<LeftoverWave> leftoverWaves;

		for (auto& convoy : world.convoys)
		{
			if (convoyIsUnderway(convoy))
			{
				continue;
			}

			if (!convoyMembersReadyToSail(world, convoy))
			{
				continue;
			}

			const HaulJob* job = world.haulJobById(convoy.haulJobId);
			if (job == nullptr)
			{
				continue;
			}

			const std::vector<long long> path =
				shortestLanePath(world, convoy.loadSystemId, job->destinationSystemId);
			if (path.size() < 2)
			{
				continue;
			}

			LeftoverWave leftover{
				.haulJobId = convoy.haulJobId,
				.empireId = convoy.empireId,
				.loadSystemId = convoy.loadSystemId };
			for (auto& truck : world.trucks)
			{
				if (truck.convoyId != convoy.id || truck.cargoAmount != 0)
				{
					continue;
				}

				leftover.truckIds.push_back(truck.id);
				truck.convoyId = 0;
			}

			if (!leftover.truckIds.empty())
			{
				leftoverWaves.push_back(std::move(leftover));
			}

			convoy.motion.route = path;
			convoy.motion.hop = 0;
			convoy.motion.progress = 0.0f;
			convoy.motion.direction = 1;
			copyConvoyMotionOntoMembers(world, convoy);
		}

		long long nextId = nextConvoyId(world);
		for (const auto& wave : leftoverWaves)
		{
			world.convoys.push_back(Convoy{
				.id = nextId,
				.haulJobId = wave.haulJobId,
				.empireId = wave.empireId,
				.loadSystemId = wave.loadSystemId });
			for (const long long truckId : wave.truckIds)
			{
				Truck* truck = world.truckById(truckId);
				if (truck != nullptr)
				{
					truck->convoyId = nextId;
				}
			}

			++nextId;
		}
	}

	void loadIfJoinedJob(WorldData& world, Truck& truck)
	{
		if (truck.haulJobId == 0 || truck.cargoAmount != 0)
		{
			return;
		}

		HaulJob* job = world.haulJobById(truck.haulJobId);
		const Convoy* convoy = world.convoyById(truck.convoyId);
		if (job == nullptr || convoy == nullptr)
		{
			return;
		}

		const long long here = truckSystemId(truck);
		if (here == 0 || here != convoy->loadSystemId)
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
		const long long convoyId = truck.convoyId;
		truck.cargoGoodId = 0;
		truck.cargoAmount = 0;
		truck.haulJobId = 0;
		truck.convoyId = 0;
		parkTruckAt(truck, job->destinationSystemId);
		removeConvoyIfNoMembers(world, convoyId);
	}

	void moveTrucks(WorldData& world)
	{
		for (auto& truck : world.trucks)
		{
			loadIfJoinedJob(world, truck);
		}

		departReadyConvoys(world);

		for (auto& truck : world.trucks)
		{
			dumpIfCarrying(world, truck);
		}

		for (auto& truck : world.trucks)
		{
			const Convoy* convoy = world.convoyById(truck.convoyId);
			if (convoy != nullptr && convoyIsUnderway(*convoy))
			{
				continue;
			}

			advanceAlongLane(world, truck.motion);
		}

		for (auto& convoy : world.convoys)
		{
			if (!convoyIsUnderway(convoy))
			{
				continue;
			}

			convoy.motion.speed = slowestMemberSpeed(world, convoy.id);
			advanceAlongLane(world, convoy.motion);
			copyConvoyMotionOntoMembers(world, convoy);
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

			const long long here = truckSystemId(truck);
			if (here == 0)
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

				if (nearestSupplySystemId(world, here, job.goodId, job.destinationSystemId) == 0)
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
		applyFactoryProduction(world);
		applyHaulJobAccepts(world);
		moveTrucks(world);
		syncHaulJobsFromUncoveredNeed(world);
		moveFleets(world);
		resolveBattles(world);
		++world.tick;
	}

}
