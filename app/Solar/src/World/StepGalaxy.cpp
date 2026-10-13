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

	void applyCivilianConsumption(WorldData& world)
	{
		for (const auto& yard : world.yards)
		{
			for (const auto& row : yard.populationConsumePerTick)
			{
				useFromStockpile(world, yard.systemId, row.goodId, row.amount);
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

	void copyConvoyMotionOntoEscortFleet(WorldData& world, const Convoy& convoy)
	{
		if (!convoyIsStanding(convoy))
		{
			return;
		}

		Fleet* fleet = world.fleetById(convoy.escortFleetId);
		if (fleet == nullptr)
		{
			return;
		}

		const float speed = fleet->motion.speed;
		fleet->motion.route = convoy.motion.route;
		fleet->motion.hop = convoy.motion.hop;
		fleet->motion.progress = convoy.motion.progress;
		fleet->motion.direction = convoy.motion.direction;
		fleet->motion.speed = speed;
		if (fleet->motion.hop >= 0 &&
			fleet->motion.hop < static_cast<int>(fleet->motion.route.size()))
		{
			fleet->systemId = fleet->motion.route[static_cast<size_t>(fleet->motion.hop)];
		}
	}

	bool convoyMembersReadyToSail(const WorldData& world, const Convoy& convoy)
	{
		bool anyLoadedAtRally = false;
		bool anyMember = false;
		bool allLoaded = true;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId != convoy.id)
			{
				continue;
			}

			if (convoyIsStanding(convoy) && !truckHasCargoHaulRole(world, truck))
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
			else
			{
				allLoaded = false;
			}
		}

		if (convoyIsStanding(convoy))
		{
			const Fleet* escort = world.fleetById(convoy.escortFleetId);
			if (escort == nullptr || escort->systemId != convoy.loadSystemId ||
				escort->motion.route.size() >= 2)
			{
				return false;
			}

			return anyMember && allLoaded &&
				countCargoHaulTrucksInConvoy(world, convoy.id) == standingDesiredTruckCount(convoy);
		}

		return anyMember && anyLoadedAtRally;
	}

	bool truckIsWithStandingEscortFleet(const Fleet& fleet, const Truck& truck)
	{
		if (isTruckAtSystem(truck, fleet.systemId))
		{
			return true;
		}

		return fleet.motion.route.size() >= 2 && truck.motion.route == fleet.motion.route &&
			truck.motion.hop == fleet.motion.hop;
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

		const Convoy* convoy = world.convoyById(convoyId);
		if (convoy != nullptr && convoyIsStanding(*convoy))
		{
			const Fleet* fleet = world.fleetById(convoy->escortFleetId);
			if (fleet != nullptr && fleet->motion.speed > 0.0f)
			{
				if (!any || fleet->motion.speed < slowest)
				{
					slowest = fleet->motion.speed;
				}
			}
		}

		return slowest;
	}

	bool standingEscortHasInGroupMembers(const WorldData& world, const Convoy& convoy, const Fleet& fleet)
	{
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId == convoy.id && truckIsWithStandingEscortFleet(fleet, truck))
			{
				return true;
			}
		}

		return false;
	}

	void applyStandingEscortFollow(WorldData& world)
	{
		for (auto& convoy : world.convoys)
		{
			if (!convoyIsStanding(convoy) || convoy.haulJobId != 0)
			{
				continue;
			}

			Fleet* fleet = world.fleetById(convoy.escortFleetId);
			if (fleet == nullptr || fleet->motion.route.size() < 2)
			{
				continue;
			}

			std::vector<long long> inGroupIds;
			float slowest = fleet->motion.speed;
			for (const auto& truck : world.trucks)
			{
				if (truck.convoyId != convoy.id || !truckIsWithStandingEscortFleet(*fleet, truck))
				{
					continue;
				}

				inGroupIds.push_back(truck.id);
				if (truck.motion.speed < slowest)
				{
					slowest = truck.motion.speed;
				}
			}

			if (inGroupIds.empty())
			{
				continue;
			}

			LaneMotion group = fleet->motion;
			group.speed = slowest;
			const bool hopDone = advanceAlongLane(world, group);
			const float fleetSpeed = fleet->motion.speed;
			fleet->motion.route = group.route;
			fleet->motion.hop = group.hop;
			fleet->motion.progress = group.progress;
			fleet->motion.direction = group.direction;
			fleet->motion.speed = fleetSpeed;
			if (hopDone && group.hop >= 0 &&
				group.hop < static_cast<int>(group.route.size()))
			{
				fleet->systemId = group.route[static_cast<size_t>(group.hop)];
				if (group.hop == static_cast<int>(group.route.size()) - 1)
				{
					fleet->motion.route.clear();
					fleet->motion.hop = 0;
				}
			}

			for (const long long truckId : inGroupIds)
			{
				Truck* truck = world.truckById(truckId);
				if (truck == nullptr)
				{
					continue;
				}

				if (fleet->motion.route.size() < 2)
				{
					parkTruckAt(*truck, fleet->systemId);
					continue;
				}

				const float truckSpeed = truck->motion.speed;
				truck->motion.route = fleet->motion.route;
				truck->motion.hop = fleet->motion.hop;
				truck->motion.progress = fleet->motion.progress;
				truck->motion.direction = fleet->motion.direction;
				truck->motion.speed = truckSpeed;
			}
		}
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

			const bool militaryPath = convoyIsStanding(convoy)
				? standingConvoyUsesMilitaryHaulPath(convoy)
				: convoyJobGroupIsMilitary(world, convoy);
			const std::vector<long long> path = haulLanePath(
				world,
				convoy.loadSystemId,
				job->destinationSystemId,
				convoy.empireId,
				convoyIsStanding(convoy),
				militaryPath);
			if (path.size() < 2)
			{
				continue;
			}

			if (!convoyIsStanding(convoy))
			{
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
			}

			convoy.motion.route = path;
			convoy.motion.hop = 0;
			convoy.motion.progress = 0.0f;
			convoy.motion.direction = 1;
			convoy.motion.speed = slowestMemberSpeed(world, convoy.id);
			convoy.expectedArrivalTick = world.tick + laneMotionRemainingTicks(world, convoy.motion);
			copyConvoyMotionOntoMembers(world, convoy);
			copyConvoyMotionOntoEscortFleet(world, convoy);
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
		const Convoy* convoy = world.convoyById(convoyId);
		const bool standing = convoy != nullptr && convoyIsStanding(*convoy);
		truck.cargoGoodId = 0;
		truck.cargoAmount = 0;
		truck.haulJobId = 0;
		if (!standing)
		{
			truck.convoyId = 0;
		}

		parkTruckAt(truck, job->destinationSystemId);
		if (!standing)
		{
			removeConvoyIfNoMembers(world, convoyId);
		}
	}

	void finishStandingRunsAfterDump(WorldData& world)
	{
		for (auto& convoy : world.convoys)
		{
			if (!convoyIsStanding(convoy) || convoy.haulJobId == 0)
			{
				continue;
			}

			bool anyAssigned = false;
			for (const auto& truck : world.trucks)
			{
				if (truck.convoyId != convoy.id)
				{
					continue;
				}

				if (truck.haulJobId != 0 || truck.cargoAmount != 0)
				{
					anyAssigned = true;
					break;
				}
			}

			if (anyAssigned)
			{
				continue;
			}

			convoy.haulJobId = 0;
			convoy.motion.route.clear();
			convoy.motion.hop = 0;
			convoy.motion.progress = 0.0f;
			Fleet* fleet = world.fleetById(convoy.escortFleetId);
			if (fleet == nullptr)
			{
				continue;
			}

			for (const auto& truck : world.trucks)
			{
				if (truck.convoyId != convoy.id)
				{
					continue;
				}

				const long long here = truckSystemId(truck);
				if (here != 0)
				{
					fleet->systemId = here;
				}

				fleet->motion.route.clear();
				fleet->motion.hop = 0;
				fleet->motion.progress = 0.0f;
				break;
			}
		}
	}

	bool convoyArrivedAtRouteEnd(const Convoy& convoy)
	{
		return convoy.motion.route.size() >= 2 &&
			convoy.motion.hop == static_cast<int>(convoy.motion.route.size()) - 1 &&
			convoy.motion.progress == 0.0f;
	}

	void parkStandingGroupAtLoadIfEmptyTransitArrived(WorldData& world)
	{
		for (auto& convoy : world.convoys)
		{
			if (!convoyIsStanding(convoy) || convoy.haulJobId == 0 || !convoyIsUnderway(convoy))
			{
				continue;
			}

			if (!convoyArrivedAtRouteEnd(convoy) || convoy.motion.route.back() != convoy.loadSystemId)
			{
				continue;
			}

			convoy.motion.route.clear();
			convoy.motion.hop = 0;
			convoy.motion.progress = 0.0f;
			for (auto& truck : world.trucks)
			{
				if (truck.convoyId == convoy.id)
				{
					parkTruckAt(truck, convoy.loadSystemId);
				}
			}

			Fleet* fleet = world.fleetById(convoy.escortFleetId);
			if (fleet != nullptr)
			{
				fleet->systemId = convoy.loadSystemId;
				fleet->motion.route.clear();
				fleet->motion.hop = 0;
				fleet->motion.progress = 0.0f;
			}
		}
	}

	void beginStandingEmptyTransitToLoad(WorldData& world)
	{
		for (auto& convoy : world.convoys)
		{
			if (!convoyIsStanding(convoy) || convoy.haulJobId == 0 || convoyIsUnderway(convoy))
			{
				continue;
			}

			Fleet* fleet = world.fleetById(convoy.escortFleetId);
			if (fleet == nullptr)
			{
				continue;
			}

			bool anyoneAway = fleet->systemId != convoy.loadSystemId || fleet->motion.route.size() >= 2;
			for (const auto& truck : world.trucks)
			{
				if (truck.convoyId != convoy.id || !truckHasCargoHaulRole(world, truck))
				{
					continue;
				}

				if (!isTruckAtSystem(truck, convoy.loadSystemId))
				{
					anyoneAway = true;
				}
			}

			if (!anyoneAway)
			{
				continue;
			}

			const std::vector<long long> path =
				shortestLanePath(world, fleet->systemId, convoy.loadSystemId);
			if (path.size() < 2)
			{
				continue;
			}

			convoy.motion.route = path;
			convoy.motion.hop = 0;
			convoy.motion.progress = 0.0f;
			convoy.motion.direction = 1;
			convoy.motion.speed = slowestMemberSpeed(world, convoy.id);
			convoy.expectedArrivalTick = world.tick + laneMotionRemainingTicks(world, convoy.motion);
			copyConvoyMotionOntoMembers(world, convoy);
			copyConvoyMotionOntoEscortFleet(world, convoy);
		}
	}

	void moveTrucks(WorldData& world)
	{
		parkStandingGroupAtLoadIfEmptyTransitArrived(world);

		for (auto& truck : world.trucks)
		{
			loadIfJoinedJob(world, truck);
		}

		departReadyConvoys(world);

		for (auto& truck : world.trucks)
		{
			dumpIfCarrying(world, truck);
		}

		finishStandingRunsAfterDump(world);
		applyStandingEscortFollow(world);

		for (auto& truck : world.trucks)
		{
			const Convoy* convoy = world.convoyById(truck.convoyId);
			if (convoy != nullptr && convoyIsUnderway(*convoy))
			{
				continue;
			}

			if (convoy != nullptr && convoyIsStanding(*convoy) && convoy->haulJobId == 0)
			{
				const Fleet* fleet = world.fleetById(convoy->escortFleetId);
				if (fleet != nullptr && fleet->motion.route.size() >= 2 &&
					truckIsWithStandingEscortFleet(*fleet, truck))
				{
					continue;
				}
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
			copyConvoyMotionOntoEscortFleet(world, convoy);
		}
	}

	void repathStandingMembersToRally(WorldData& world)
	{
		for (auto& truck : world.trucks)
		{
			if (truck.haulJobId != 0)
			{
				continue;
			}

			const Convoy* convoy = world.convoyById(truck.convoyId);
			if (convoy == nullptr || !convoyIsStanding(*convoy) || convoyIsUnderway(*convoy))
			{
				continue;
			}

			pathTruckToSystem(world, truck, standingRallySystemId(world, *convoy));
		}
	}

	void applyStandingConvoyJoins(WorldData& world)
	{
		std::vector<GameCommand> joins;
		for (const auto& truck : world.trucks)
		{
			if (truck.cargoAmount != 0 || truck.haulJobId != 0 || truck.convoyId != 0)
			{
				continue;
			}

			if (truckSystemId(truck) == 0)
			{
				continue;
			}

			long long bestConvoyId = 0;
			for (const auto& convoy : world.convoys)
			{
				if (!convoyIsStanding(convoy) || convoy.empireId != truck.empireId)
				{
					continue;
				}

				if (!standingConvoyHasJoinSlotForTruck(world, convoy, truck))
				{
					continue;
				}

				if (bestConvoyId == 0 || convoy.id < bestConvoyId)
				{
					bestConvoyId = convoy.id;
				}
			}

			if (bestConvoyId != 0)
			{
				joins.push_back(GameCommand::joinStandingConvoy(truck.id, bestConvoyId, CommandSource::Ai));
			}
		}

		std::sort(
			joins.begin(),
			joins.end(),
			[](const GameCommand& a, const GameCommand& b)
			{
				return std::get<JoinStandingConvoy>(a.payload).truckId <
					std::get<JoinStandingConvoy>(b.payload).truckId;
			});

		for (const auto& command : joins)
		{
			execute(world, command);
		}
	}

	void applyStandingConvoyHaulAccepts(WorldData& world)
	{
		std::vector<GameCommand> accepts;
		for (const auto& convoy : world.convoys)
		{
			if (!convoyIsStanding(convoy) || convoy.haulJobId != 0)
			{
				continue;
			}

			if (countCargoHaulTrucksInConvoy(world, convoy.id) != standingDesiredTruckCount(convoy))
			{
				continue;
			}

			const Fleet* fleet = world.fleetById(convoy.escortFleetId);
			if (fleet == nullptr)
			{
				continue;
			}

			const long long rally = fleetRallySystemId(*fleet);
			long long bestJobId = 0;
			for (const auto& job : world.haulJobs)
			{
				if (job.empireId != convoy.empireId)
				{
					continue;
				}

				const long long truckload = truckloadForGood(job.goodId);
				const long long reserved = standingDesiredTruckCount(convoy) * truckload;
				if (job.amount < reserved)
				{
					continue;
				}

				const long long loadSystemId = nearestSupplySystemId(
					world,
					rally,
					job.goodId,
					job.destinationSystemId,
					convoy.empireId,
					true,
					standingConvoyUsesMilitaryHaulPath(convoy));
				if (loadSystemId == 0)
				{
					continue;
				}

				const StarSystem* supply = world.systemById(loadSystemId);
				if (supply == nullptr || getStockpileAmount(*supply, job.goodId) < reserved)
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
				accepts.push_back(
					GameCommand::acceptStandingConvoyHaulJob(convoy.id, bestJobId, CommandSource::Ai));
			}
		}

		std::sort(
			accepts.begin(),
			accepts.end(),
			[](const GameCommand& a, const GameCommand& b)
			{
				return std::get<AcceptStandingConvoyHaulJob>(a.payload).convoyId <
					std::get<AcceptStandingConvoyHaulJob>(b.payload).convoyId;
			});

		for (const auto& command : accepts)
		{
			execute(world, command);
		}
	}

	void applyHaulJobAccepts(WorldData& world)
	{
		std::vector<GameCommand> accepts;
		for (const auto& truck : world.trucks)
		{
			if (truck.cargoAmount != 0 || truck.haulJobId != 0 || truck.convoyId != 0)
			{
				continue;
			}

			const long long here = truckSystemId(truck);
			if (here == 0)
			{
				continue;
			}

			if (!truckHasCargoHaulRole(world, truck))
			{
				continue;
			}

			long long bestJobId = 0;
			long long bestRisk = -1;
			const bool militaryPath = truckUsesMilitaryHaulPath(world, truck);
			for (const auto& job : world.haulJobs)
			{
				if (job.empireId != truck.empireId)
				{
					continue;
				}

				const StarSystem* destination = world.systemById(job.destinationSystemId);
				if (!militaryPath && destination != nullptr && destination->empireId != 0 &&
					civilianHaulIsForbidden(world, truck.empireId, destination->empireId))
				{
					continue;
				}

				if (job.amount < truckloadForGood(job.goodId))
				{
					continue;
				}

				if (nearestSupplySystemId(
						world,
						here,
						job.goodId,
						job.destinationSystemId,
						truck.empireId,
						false,
						militaryPath) == 0)
				{
					continue;
				}

				const long long civilianSupply = nearestSupplySystemId(
					world,
					here,
					job.goodId,
					job.destinationSystemId,
					truck.empireId);
				const bool destForbidden = destination != nullptr && destination->empireId != 0 &&
					civilianHaulIsForbidden(world, truck.empireId, destination->empireId);
				const long long risk = (civilianSupply == 0 || destForbidden) ? 1 : 0;
				if (militaryPath)
				{
					if (bestJobId == 0 || risk > bestRisk || (risk == bestRisk && job.id < bestJobId))
					{
						bestRisk = risk;
						bestJobId = job.id;
					}
				}
				else if (bestJobId == 0 || job.id < bestJobId)
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
			const Convoy* escort = world.convoyById(fleet.escortConvoyId);
			if (escort != nullptr && convoyIsUnderway(*escort))
			{
				continue;
			}

			if (escort != nullptr && convoyIsStanding(*escort) && escort->haulJobId == 0 &&
				fleet.motion.route.size() >= 2 &&
				standingEscortHasInGroupMembers(world, *escort, fleet))
			{
				continue;
			}

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
		applyCivilianConsumption(world);
		applyYardSlipConstruction(world);
		applyYardHullConstruction(world);
		applyYardMagazineRestock(world);
		applyTenderLoadFromYard(world);
		applyTenderUnloadIntoMagazines(world);
		applyStandingConvoyJoins(world);
		applyHaulJobAccepts(world);
		applyStandingConvoyHaulAccepts(world);
		beginStandingEmptyTransitToLoad(world);
		repathStandingMembersToRally(world);
		moveTrucks(world);
		syncHaulJobsFromUncoveredNeed(world);
		moveFleets(world);
		resolveBattles(world);
		++world.tick;
	}

}
