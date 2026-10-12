#include <World/WorldData.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <algorithm>
#include <utility>

namespace slr
{

	const Empire* WorldData::empireById(long long id) const
	{
		for (const auto& empire : empires)
		{
			if (empire.id == id)
			{
				return &empire;
			}
		}

		return nullptr;
	}

	Empire* WorldData::empireById(long long id)
	{
		return const_cast<Empire*>(static_cast<const WorldData*>(this)->empireById(id));
	}

	const StarSystem* WorldData::systemById(long long id) const
	{
		for (const auto& system : systems)
		{
			if (system.id == id)
			{
				return &system;
			}
		}

		return nullptr;
	}

	StarSystem* WorldData::systemById(long long id)
	{
		return const_cast<StarSystem*>(static_cast<const WorldData*>(this)->systemById(id));
	}

	const ShipClass* WorldData::shipClassById(long long id) const
	{
		for (const auto& shipClass : shipClasses)
		{
			if (shipClass.id == id)
			{
				return &shipClass;
			}
		}

		return nullptr;
	}

	ShipClass* WorldData::shipClassById(long long id)
	{
		return const_cast<ShipClass*>(static_cast<const WorldData*>(this)->shipClassById(id));
	}

	const Fleet* WorldData::fleetById(long long id) const
	{
		for (const auto& fleet : fleets)
		{
			if (fleet.id == id)
			{
				return &fleet;
			}
		}

		return nullptr;
	}

	Fleet* WorldData::fleetById(long long id)
	{
		return const_cast<Fleet*>(static_cast<const WorldData*>(this)->fleetById(id));
	}

	const HaulJob* WorldData::haulJobById(long long id) const
	{
		for (const auto& job : haulJobs)
		{
			if (job.id == id)
			{
				return &job;
			}
		}

		return nullptr;
	}

	HaulJob* WorldData::haulJobById(long long id)
	{
		return const_cast<HaulJob*>(static_cast<const WorldData*>(this)->haulJobById(id));
	}

	const Convoy* WorldData::convoyById(long long id) const
	{
		for (const auto& convoy : convoys)
		{
			if (convoy.id == id)
			{
				return &convoy;
			}
		}

		return nullptr;
	}

	Convoy* WorldData::convoyById(long long id)
	{
		return const_cast<Convoy*>(static_cast<const WorldData*>(this)->convoyById(id));
	}

	const Truck* WorldData::truckById(long long id) const
	{
		for (const auto& truck : trucks)
		{
			if (truck.id == id)
			{
				return &truck;
			}
		}

		return nullptr;
	}

	Truck* WorldData::truckById(long long id)
	{
		return const_cast<Truck*>(static_cast<const WorldData*>(this)->truckById(id));
	}

	const EngagementPolicy* WorldData::engagementPolicyById(long long id) const
	{
		for (const auto& policy : engagementPolicies)
		{
			if (policy.id == id)
			{
				return &policy;
			}
		}

		return nullptr;
	}

	EngagementPolicy* WorldData::engagementPolicyById(long long id)
	{
		return const_cast<EngagementPolicy*>(static_cast<const WorldData*>(this)->engagementPolicyById(id));
	}

	bool ruleOfEngagementOpensFire(RuleOfEngagement rule)
	{
		return rule == RuleOfEngagement::FireIfEnemySeen;
	}

	const char* ruleOfEngagementName(RuleOfEngagement rule)
	{
		switch (rule)
		{
		case RuleOfEngagement::FireIfEnemySeen:
			return "fire if enemy seen";
		case RuleOfEngagement::DoNotFireUnlessFiredUpon:
			return "do not fire unless fired upon";
		}

		return "unknown";
	}

	const EngagementPolicy* activeEngagementPolicy(const WorldData& world, const Fleet& fleet)
	{
		if (fleet.policyId != 0)
		{
			const EngagementPolicy* overridePolicy = world.engagementPolicyById(fleet.policyId);
			if (overridePolicy != nullptr && overridePolicy->empireId == fleet.empireId)
			{
				return overridePolicy;
			}
		}

		const Empire* empire = world.empireById(fleet.empireId);
		if (empire == nullptr || empire->defaultPolicyId == 0)
		{
			return nullptr;
		}

		const EngagementPolicy* defaultPolicy = world.engagementPolicyById(empire->defaultPolicyId);
		if (defaultPolicy == nullptr || defaultPolicy->empireId != fleet.empireId)
		{
			return nullptr;
		}

		return defaultPolicy;
	}

	bool anyFleetOpensFire(const WorldData& world, const std::vector<long long>& fleetIds)
	{
		for (const long long fleetId : fleetIds)
		{
			const Fleet* fleet = world.fleetById(fleetId);
			if (fleet == nullptr)
			{
				continue;
			}

			const EngagementPolicy* policy = activeEngagementPolicy(world, *fleet);
			if (policy != nullptr && ruleOfEngagementOpensFire(policy->ruleOfEngagement))
			{
				return true;
			}
		}

		return false;
	}

	long long getStockpileAmount(const StarSystem& system, long long goodId)
	{
		for (const auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				return stock.goodAmount;
			}
		}

		return 0;
	}

	long long locatedGoodAmount(const WorldData& world, long long goodId)
	{
		long long total = 0;
		for (const auto& system : world.systems)
		{
			total += getStockpileAmount(system, goodId);
		}

		for (const auto& truck : world.trucks)
		{
			if (truck.cargoGoodId == goodId)
			{
				total += truck.cargoAmount;
			}
		}

		return total;
	}

	long long getInbound(const StarSystem& system, long long goodId)
	{
		for (const auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				return stock.inbound;
			}
		}

		return 0;
	}

	long long getNeed(const StarSystem& system, long long goodId)
	{
		for (const auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				return stock.need;
			}
		}

		return 0;
	}

	long long uncoveredNeed(const StarSystem& system, long long goodId)
	{
		const long long inbound = getInbound(system, goodId);
		const long long need = getNeed(system, goodId);
		if (need <= inbound)
		{
			return 0;
		}

		return need - inbound;
	}

	long long quantityForGood(const std::vector<GoodQuantity>& rows, long long goodId)
	{
		long long total = 0;
		for (const auto& row : rows)
		{
			if (row.goodId == goodId)
			{
				total += row.amount;
			}
		}

		return total;
	}

	long long mineOutputPerTick(const Mine& mine, long long goodId)
	{
		long long total = 0;
		for (const auto& output : mine.outputsPerTick)
		{
			if (output.goodId == goodId)
			{
				total += output.amountPerTick;
			}
		}

		return total;
	}

	long long mineMaxStockpile(const Mine& mine, long long goodId)
	{
		long long total = 0;
		for (const auto& output : mine.outputsPerTick)
		{
			if (output.goodId == goodId)
			{
				total += output.maxStockpile;
			}
		}

		return total;
	}

	long long consumerDesiredPile(const WorldData& world, long long systemId, long long goodId)
	{
		long long desired = 0;
		for (const auto& yard : world.yards)
		{
			if (yard.systemId == systemId)
			{
				desired += quantityForGood(yard.desiredPiles, goodId);
				desired += quantityForGood(yard.populationReservedPiles, goodId);
			}
		}

		for (const auto& factory : world.factories)
		{
			if (factory.systemId == systemId)
			{
				desired += quantityForGood(factory.desiredPiles, goodId);
			}
		}

		return desired;
	}

	long long stockpileAvailableForYardConsume(const WorldData& world, long long systemId, long long goodId)
	{
		long long reserved = 0;
		for (const auto& yard : world.yards)
		{
			if (yard.systemId == systemId)
			{
				reserved += quantityForGood(yard.populationReservedPiles, goodId);
			}
		}

		const StarSystem* system = world.systemById(systemId);
		if (system == nullptr)
		{
			return 0;
		}

		return std::max(0LL, getStockpileAmount(*system, goodId) - reserved);
	}

	void postNeedFromConsumerShortfall(WorldData& world, StarSystem& system, long long goodId)
	{
		const long long desired = consumerDesiredPile(world, system.id, goodId);
		const long long stockpile = getStockpileAmount(system, goodId);
		long long shortfall = desired - stockpile;
		if (shortfall < 0)
		{
			shortfall = 0;
		}

		const long long posted = (shortfall >= truckloadForGood(goodId)) ? shortfall : 0;
		for (auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				stock.need = posted;
				return;
			}
		}

		if (posted == 0)
		{
			return;
		}

		system.stocks.push_back(GoodStock{ .goodId = goodId, .need = posted });
	}

	void postNeedsFromConsumerShortfalls(WorldData& world, StarSystem& system)
	{
		for (const auto& yard : world.yards)
		{
			if (yard.systemId != system.id)
			{
				continue;
			}

			for (const auto& pile : yard.desiredPiles)
			{
				postNeedFromConsumerShortfall(world, system, pile.goodId);
			}

			for (const auto& pile : yard.populationReservedPiles)
			{
				postNeedFromConsumerShortfall(world, system, pile.goodId);
			}

			for (const auto& pile : yard.populationConsumePerTick)
			{
				postNeedFromConsumerShortfall(world, system, pile.goodId);
			}
		}

		for (const auto& factory : world.factories)
		{
			if (factory.systemId != system.id)
			{
				continue;
			}

			for (const auto& pile : factory.desiredPiles)
			{
				postNeedFromConsumerShortfall(world, system, pile.goodId);
			}
		}
	}

	bool useFromStockpile(WorldData& world, long long systemId, long long goodId, long long amount)
	{
		if (amount <= 0)
		{
			return false;
		}

		StarSystem* system = world.systemById(systemId);
		if (system == nullptr)
		{
			return false;
		}

		for (auto& stock : system->stocks)
		{
			if (stock.goodId == goodId && stock.goodAmount >= amount)
			{
				stock.goodAmount -= amount;
				stock.deferredNeed += amount;
				stock.deferredNeedHeldOneTick = false;
				return true;
			}
		}

		return false;
	}

	void applyDeferredNeedPosts(WorldData& world)
	{
		for (auto& system : world.systems)
		{
			for (auto& stock : system.stocks)
			{
				const long long truckload = truckloadForGood(stock.goodId);
				if (stock.deferredNeedHeldOneTick && stock.deferredNeed >= truckload)
				{
					stock.need += stock.deferredNeed;
					stock.deferredNeed = 0;
					stock.deferredNeedHeldOneTick = false;
				}
				else if (stock.deferredNeed > 0)
				{
					stock.deferredNeedHeldOneTick = true;
				}
			}
		}
	}

	bool isTruckAtSystem(const Truck& truck, long long systemId)
	{
		if (truck.motion.progress != 0.0f || truck.motion.hop < 0 ||
			truck.motion.hop >= static_cast<int>(truck.motion.route.size()))
		{
			return false;
		}

		return truck.motion.route[static_cast<size_t>(truck.motion.hop)] == systemId;
	}

	long long countTrucksAtSystem(const WorldData& world, long long systemId)
	{
		long long count = 0;
		for (const auto& truck : world.trucks)
		{
			if (isTruckAtSystem(truck, systemId))
			{
				++count;
			}
		}

		return count;
	}

	long long fleetShipCount(const Fleet& fleet)
	{
		long long count = 0;
		for (const auto& row : fleet.composition)
		{
			count += row.count;
		}

		return count;
	}

	bool fleetIsParked(const Fleet& fleet)
	{
		return !(fleet.motion.progress > 0.0f);
	}

	long long classCountFor(const std::vector<ClassCount>& composition, long long classId)
	{
		for (const auto& row : composition)
		{
			if (row.classId == classId)
			{
				return row.count;
			}
		}

		return 0;
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

	const HaulJob* haulJobFor(const WorldData& world, long long destinationSystemId, long long goodId)
	{
		for (const auto& job : world.haulJobs)
		{
			if (job.destinationSystemId == destinationSystemId && job.goodId == goodId)
			{
				return &job;
			}
		}

		return nullptr;
	}

	long long truckSystemId(const Truck& truck)
	{
		if (truck.motion.progress == 0.0f && truck.motion.hop >= 0 &&
			truck.motion.hop < static_cast<int>(truck.motion.route.size()))
		{
			return truck.motion.route[static_cast<size_t>(truck.motion.hop)];
		}

		long long fromId = 0;
		long long toId = 0;
		if (hopEndpoints(truck.motion, fromId, toId))
		{
			return fromId;
		}

		return 0;
	}

	namespace
	{

		bool systemCanSupplyGood(const WorldData& world, const StarSystem& system, long long goodId)
		{
			if (getStockpileAmount(system, goodId) >= truckloadForGood(goodId))
			{
				return true;
			}

			for (const auto& mine : world.mines)
			{
				if (mine.systemId == system.id && mineOutputPerTick(mine, goodId) > 0)
				{
					return true;
				}
			}

			return false;
		}

		long long empireIdForHaulDestination(const WorldData& world, long long systemId)
		{
			for (const auto& yard : world.yards)
			{
				if (yard.systemId == systemId)
				{
					return yard.empireId;
				}
			}

			for (const auto& factory : world.factories)
			{
				if (factory.systemId == systemId)
				{
					return factory.empireId;
				}
			}

			return 0;
		}

		long long emptyAssignedOnJob(const WorldData& world, long long jobId, long long goodId)
		{
			const long long truckload = truckloadForGood(goodId);
			long long reserved = 0;
			for (const auto& truck : world.trucks)
			{
				if (truck.haulJobId == jobId && truck.cargoAmount == 0)
				{
					reserved += truckload;
				}
			}

			return reserved;
		}

		long long remainingAcceptRoom(const WorldData& world, const StarSystem& destination, const HaulJob& job)
		{
			const long long uncovered = uncoveredNeed(destination, job.goodId);
			const long long reserved = emptyAssignedOnJob(world, job.id, job.goodId);
			if (uncovered <= reserved)
			{
				return 0;
			}

			return uncovered - reserved;
		}

		bool anyTruckAssignedToJob(const WorldData& world, long long jobId)
		{
			for (const auto& truck : world.trucks)
			{
				if (truck.haulJobId == jobId)
				{
					return true;
				}
			}

			return false;
		}

	}

	long long nearestSupplySystemId(
		const WorldData& world,
		long long fromSystemId,
		long long goodId,
		long long destinationSystemId,
		long long viewerEmpireId,
		bool seesEscort,
		bool isMilitary)
	{
		auto pick = [&](bool allowDestination) -> long long
		{
			long long bestId = 0;
			long long bestCost = 0;
			bool found = false;
			for (const auto& system : world.systems)
			{
				if (!allowDestination && system.id == destinationSystemId)
				{
					continue;
				}

				if (!systemCanSupplyGood(world, system, goodId))
				{
					continue;
				}

				const long long toSupply = haulLanePathCost(
					world,
					fromSystemId,
					system.id,
					viewerEmpireId,
					seesEscort,
					isMilitary);
				if (toSupply < 0)
				{
					continue;
				}

				const long long toDest = haulLanePathCost(
					world,
					system.id,
					destinationSystemId,
					viewerEmpireId,
					seesEscort,
					isMilitary);
				if (toDest < 0)
				{
					continue;
				}

				const long long cost = toSupply + toDest;
				if (!found || cost < bestCost || (cost == bestCost && system.id < bestId))
				{
					found = true;
					bestCost = cost;
					bestId = system.id;
				}
			}

			return bestId;
		};

		const long long skipDest = pick(false);
		if (skipDest != 0)
		{
			return skipDest;
		}

		return pick(true);
	}

	void parkTruckAt(Truck& truck, long long systemId)
	{
		truck.motion.route = { systemId };
		truck.motion.hop = 0;
		truck.motion.progress = 0.0f;
		truck.motion.direction = 1;
	}

	void syncHaulJobsFromUncoveredNeed(WorldData& world)
	{
		long long nextId = 1;
		for (const auto& job : world.haulJobs)
		{
			if (job.id >= nextId)
			{
				nextId = job.id + 1;
			}
		}

		for (const auto& system : world.systems)
		{
			for (const auto& stock : system.stocks)
			{
				HaulJob* existing = nullptr;
				for (auto& job : world.haulJobs)
				{
					if (job.destinationSystemId == system.id && job.goodId == stock.goodId)
					{
						existing = &job;
						break;
					}
				}

				const long long truckload = truckloadForGood(stock.goodId);
				long long remaining = 0;
				if (existing != nullptr)
				{
					remaining = remainingAcceptRoom(world, system, *existing);
				}
				else
				{
					remaining = uncoveredNeed(system, stock.goodId);
				}

				const bool assigned = existing != nullptr && anyTruckAssignedToJob(world, existing->id);
				if (remaining < truckload && !assigned)
				{
					continue;
				}

				const long long empireId = empireIdForHaulDestination(world, system.id);
				if (existing != nullptr)
				{
					existing->amount = remaining;
					if (existing->empireId == 0)
					{
						existing->empireId = empireId;
					}
				}
				else if (remaining >= truckload)
				{
					world.haulJobs.push_back(HaulJob{
						.id = nextId,
						.destinationSystemId = system.id,
						.empireId = empireId,
						.goodId = stock.goodId,
						.amount = remaining });
					++nextId;
				}
			}
		}

		world.haulJobs.erase(
			std::remove_if(
				world.haulJobs.begin(),
				world.haulJobs.end(),
				[&world](const HaulJob& job)
				{
					const StarSystem* system = world.systemById(job.destinationSystemId);
					if (system == nullptr)
					{
						return true;
					}

					if (anyTruckAssignedToJob(world, job.id))
					{
						return false;
					}

					return remainingAcceptRoom(world, *system, job) < truckloadForGood(job.goodId);
				}),
			world.haulJobs.end());
	}

	long long truckCountToCarryAmountInOneGo(long long amount, long long goodId)
	{
		if (amount <= 0)
		{
			return 0;
		}

		const long long truckload = truckloadForGood(goodId);
		return (amount + truckload - 1) / truckload;
	}

	bool convoyCarriesAmountInOneGo(long long truckCount, long long amount, long long goodId)
	{
		if (truckCount <= 0 || amount <= 0)
		{
			return false;
		}

		return truckCount * truckloadForGood(goodId) >= amount;
	}

	bool convoyIsUnderway(const Convoy& convoy)
	{
		return convoy.motion.route.size() >= 2;
	}

	bool convoyIsStanding(const Convoy& convoy)
	{
		return convoy.escortFleetId != 0;
	}

	bool standingConvoyUsesMilitaryHaulPath(const Convoy& convoy)
	{
		return convoyIsStanding(convoy) && convoy.desiredCivilianTruckCount == 0 &&
			convoy.desiredMilitaryTruckCount > 0;
	}

	long long standingDesiredTruckCount(const Convoy& convoy)
	{
		return convoy.desiredCivilianTruckCount + convoy.desiredMilitaryTruckCount;
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

	long long fleetRallySystemId(const Fleet& fleet)
	{
		if (fleet.motion.route.size() >= 2)
		{
			return fleet.motion.route.back();
		}

		return fleet.systemId;
	}

	long long standingRallySystemId(const WorldData& world, const Convoy& convoy)
	{
		const Fleet* fleet = world.fleetById(convoy.escortFleetId);
		if (fleet != nullptr)
		{
			return fleetRallySystemId(*fleet);
		}

		return convoy.loadSystemId;
	}

	bool pathTruckToSystem(WorldData& world, Truck& truck, long long systemId)
	{
		const long long here = truckSystemId(truck);
		if (here == 0 || world.systemById(systemId) == nullptr)
		{
			return false;
		}

		if (here == systemId)
		{
			parkTruckAt(truck, systemId);
			return true;
		}

		if (!truck.motion.route.empty() && truck.motion.route.back() == systemId)
		{
			return true;
		}

		bool seesEscort = false;
		bool standingMilitaryPath = false;
		if (const Convoy* convoy = world.convoyById(truck.convoyId))
		{
			seesEscort = convoyIsStanding(*convoy);
			standingMilitaryPath = standingConvoyUsesMilitaryHaulPath(*convoy);
		}

		const bool isMilitary = truck.isMilitary && (!seesEscort || standingMilitaryPath);
		const std::vector<long long> path =
			haulLanePath(world, here, systemId, truck.empireId, seesEscort, isMilitary);
		if (path.size() < 2)
		{
			return false;
		}

		truck.motion.route = path;
		truck.motion.hop = 0;
		truck.motion.progress = 0.0f;
		truck.motion.direction = 1;
		return true;
	}

	bool convoyJobGroupIsMilitary(const WorldData& world, const Convoy& convoy)
	{
		if (convoyIsStanding(convoy))
		{
			return false;
		}

		bool any = false;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId != convoy.id)
			{
				continue;
			}

			if (!truck.isMilitary)
			{
				return false;
			}

			any = true;
		}

		return any;
	}

	Convoy* openConvoyForHaulJob(WorldData& world, long long haulJobId, bool isMilitary)
	{
		for (auto& convoy : world.convoys)
		{
			if (convoyIsStanding(convoy))
			{
				continue;
			}

			if (convoy.haulJobId != haulJobId || convoyIsUnderway(convoy))
			{
				continue;
			}

			if (countTrucksInConvoy(world, convoy.id) == 0)
			{
				continue;
			}

			if (convoyJobGroupIsMilitary(world, convoy) != isMilitary)
			{
				continue;
			}

			return &convoy;
		}

		return nullptr;
	}

	long long countTrucksInConvoy(const WorldData& world, long long convoyId)
	{
		long long count = 0;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId == convoyId)
			{
				++count;
			}
		}

		return count;
	}

	long long countMilitaryTrucksInConvoy(const WorldData& world, long long convoyId)
	{
		long long count = 0;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId == convoyId && truck.isMilitary)
			{
				++count;
			}
		}

		return count;
	}

	long long countCivilianTrucksInConvoy(const WorldData& world, long long convoyId)
	{
		return countTrucksInConvoy(world, convoyId) - countMilitaryTrucksInConvoy(world, convoyId);
	}

	bool standingConvoyHasJoinSlotForTruck(
		const WorldData& world,
		const Convoy& convoy,
		const Truck& truck)
	{
		if (!convoyIsStanding(convoy))
		{
			return false;
		}

		if (truck.isMilitary)
		{
			return countMilitaryTrucksInConvoy(world, convoy.id) < convoy.desiredMilitaryTruckCount;
		}

		return countCivilianTrucksInConvoy(world, convoy.id) < convoy.desiredCivilianTruckCount;
	}

	long long countTrucksInConvoyPresent(const WorldData& world, const Convoy& convoy)
	{
		if (convoyIsUnderway(convoy))
		{
			return countTrucksInConvoy(world, convoy.id);
		}

		const long long presentAt = convoyIsStanding(convoy) && convoy.haulJobId == 0
			? standingRallySystemId(world, convoy)
			: convoy.loadSystemId;
		long long count = 0;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId == convoy.id && isTruckAtSystem(truck, presentAt))
			{
				++count;
			}
		}

		return count;
	}

	long long emptyTrucksInConvoy(const WorldData& world, long long convoyId)
	{
		long long count = 0;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId == convoyId && truck.cargoAmount == 0)
			{
				++count;
			}
		}

		return count;
	}

	bool haulWaveHasStockpileForAnotherTruck(
		const WorldData& world,
		long long loadSystemId,
		long long goodId,
		long long assemblingConvoyId)
	{
		const StarSystem* supply = world.systemById(loadSystemId);
		if (supply == nullptr)
		{
			return false;
		}

		const long long truckload = truckloadForGood(goodId);
		long long reserved = 0;
		for (const auto& truck : world.trucks)
		{
			if (truck.cargoAmount != 0 || truck.haulJobId == 0)
			{
				continue;
			}

			const HaulJob* job = world.haulJobById(truck.haulJobId);
			if (job == nullptr || job->goodId != goodId)
			{
				continue;
			}

			const Convoy* convoy = world.convoyById(truck.convoyId);
			if (convoy == nullptr || convoyIsUnderway(*convoy) ||
				convoy->loadSystemId != loadSystemId)
			{
				continue;
			}

			reserved += truckload;
		}

		return getStockpileAmount(*supply, goodId) >= reserved + truckload;
	}

	long long convoyCargoOrReservedAmount(const WorldData& world, const Convoy& convoy)
	{
		long long amount = 0;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId != convoy.id)
			{
				continue;
			}

			if (truck.cargoAmount > 0)
			{
				amount += truck.cargoAmount;
			}
			else
			{
				const HaulJob* job = world.haulJobById(convoy.haulJobId);
				if (job != nullptr)
				{
					amount += truckloadForGood(job->goodId);
				}
			}
		}

		return amount;
	}

	void removeConvoyIfNoMembers(WorldData& world, long long convoyId)
	{
		if (convoyId == 0 || countTrucksInConvoy(world, convoyId) > 0)
		{
			return;
		}

		const Convoy* convoy = world.convoyById(convoyId);
		if (convoy != nullptr && convoyIsStanding(*convoy))
		{
			return;
		}

		world.convoys.erase(
			std::remove_if(
				world.convoys.begin(),
				world.convoys.end(),
				[convoyId](const Convoy& convoy) { return convoy.id == convoyId; }),
			world.convoys.end());
	}

	long long systemOwnershipScore(const WorldData& world, long long systemId)
	{
		long long score = 0;
		for (const auto& mine : world.mines)
		{
			if (mine.systemId == systemId)
			{
				++score;
			}
		}

		for (const auto& factory : world.factories)
		{
			if (factory.systemId == systemId)
			{
				++score;
			}
		}

		bool population = false;
		for (const auto& yard : world.yards)
		{
			if (yard.systemId != systemId)
			{
				continue;
			}

			++score;
			if (!yard.populationConsumePerTick.empty())
			{
				population = true;
			}
		}

		if (population)
		{
			++score;
		}

		return score;
	}

	SystemOwnershipType systemOwnershipType(const WorldData& world, long long systemId)
	{
		for (const auto& yard : world.yards)
		{
			if (yard.systemId == systemId && !yard.populationConsumePerTick.empty())
			{
				return SystemOwnershipType::Populated;
			}
		}

		for (const auto& factory : world.factories)
		{
			if (factory.systemId == systemId)
			{
				return SystemOwnershipType::Industrial;
			}
		}

		for (const auto& yard : world.yards)
		{
			if (yard.systemId == systemId)
			{
				return SystemOwnershipType::Industrial;
			}
		}

		for (const auto& mine : world.mines)
		{
			if (mine.systemId == systemId)
			{
				return SystemOwnershipType::Extraction;
			}
		}

		return SystemOwnershipType::Empty;
	}

	const char* systemOwnershipTypeName(SystemOwnershipType type)
	{
		switch (type)
		{
		case SystemOwnershipType::Populated:
			return "populated";
		case SystemOwnershipType::Industrial:
			return "industrial";
		case SystemOwnershipType::Extraction:
			return "extraction";
		case SystemOwnershipType::Empty:
			return "empty";
		}

		return "unknown";
	}

	long long systemPresenceScore(const WorldData& world, long long systemId)
	{
		long long score = 0;
		for (const auto& fleet : world.fleets)
		{
			if (fleet.systemId == systemId && fleetIsParked(fleet) && fleetShipCount(fleet) > 0)
			{
				++score;
			}
		}

		return score;
	}

	bool systemHasParkedHostileFleet(const WorldData& world, long long systemId, long long viewerEmpireId)
	{
		for (const auto& fleet : world.fleets)
		{
			if (fleet.systemId != systemId || fleet.empireId == viewerEmpireId)
			{
				continue;
			}

			if (fleetIsParked(fleet) && fleetShipCount(fleet) > 0)
			{
				return true;
			}
		}

		return false;
	}

	bool systemIsAdjacentToParkedHostileFleet(
		const WorldData& world,
		long long systemId,
		long long viewerEmpireId)
	{
		for (const auto& lane : world.hyperlanes)
		{
			if (lane.a == systemId && systemHasParkedHostileFleet(world, lane.b, viewerEmpireId))
			{
				return true;
			}

			if (lane.b == systemId && systemHasParkedHostileFleet(world, lane.a, viewerEmpireId))
			{
				return true;
			}
		}

		return false;
	}

	bool hazardEventIsHot(long long eventTick, long long worldTick)
	{
		return worldTick - eventTick < CivilianHazardFadeTicks;
	}

	bool systemHasHazardHeat(const WorldData& world, long long systemId, long long viewerEmpireId)
	{
		for (const auto& report : world.battleReports)
		{
			if (report.systemId == systemId && hazardEventIsHot(report.tick, world.tick))
			{
				return true;
			}
		}

		for (const auto& loss : world.civilianLosses)
		{
			if (loss.systemId == systemId && loss.victimEmpireId == viewerEmpireId &&
				hazardEventIsHot(loss.tick, world.tick))
			{
				return true;
			}
		}

		for (const auto& convoy : world.convoys)
		{
			if (convoy.empireId != viewerEmpireId || !convoyIsUnderway(convoy))
			{
				continue;
			}

			if (convoy.expectedArrivalTick <= 0 || world.tick <= convoy.expectedArrivalTick)
			{
				continue;
			}

			if (convoy.motion.hop < 0)
			{
				continue;
			}

			for (int i = convoy.motion.hop; i < static_cast<int>(convoy.motion.route.size()); ++i)
			{
				if (convoy.motion.route[static_cast<size_t>(i)] == systemId)
				{
					return true;
				}
			}
		}

		return false;
	}

	bool systemIsAdjacentToHazardHeat(
		const WorldData& world,
		long long systemId,
		long long viewerEmpireId)
	{
		for (const auto& lane : world.hyperlanes)
		{
			if (lane.a == systemId && systemHasHazardHeat(world, lane.b, viewerEmpireId))
			{
				return true;
			}

			if (lane.b == systemId && systemHasHazardHeat(world, lane.a, viewerEmpireId))
			{
				return true;
			}
		}

		return false;
	}

	bool empiresAreAtWar(const WorldData& world, long long empireA, long long empireB)
	{
		if (empireA == 0 || empireB == 0 || empireA == empireB)
		{
			return false;
		}

		if (empireA > empireB)
		{
			std::swap(empireA, empireB);
		}

		for (const auto& row : world.empireWars)
		{
			if (row.empireA == empireA && row.empireB == empireB)
			{
				return row.atWar;
			}
		}

		return false;
	}

	void declareWarBetweenEmpires(WorldData& world, long long empireA, long long empireB)
	{
		if (empireA == 0 || empireB == 0 || empireA == empireB)
		{
			return;
		}

		if (empireA > empireB)
		{
			std::swap(empireA, empireB);
		}

		for (auto& row : world.empireWars)
		{
			if (row.empireA == empireA && row.empireB == empireB)
			{
				row.atWar = true;
				return;
			}
		}

		world.empireWars.push_back(EmpireWar{ .empireA = empireA, .empireB = empireB, .atWar = true });
	}

	bool systemClaimBlocksCivilian(
		const WorldData& world,
		long long systemId,
		long long viewerEmpireId)
	{
		const StarSystem* system = world.systemById(systemId);
		if (system == nullptr || system->empireId == 0 || system->empireId == viewerEmpireId)
		{
			return false;
		}

		return empiresAreAtWar(world, viewerEmpireId, system->empireId);
	}

	bool civilianHaulIsForbidden(
		const WorldData& world,
		long long empireId,
		long long forbiddenEmpireId)
	{
		if (empireId == 0 || forbiddenEmpireId == 0 || empireId == forbiddenEmpireId)
		{
			return false;
		}

		for (const auto& row : world.civilianHaulForbids)
		{
			if (row.empireId == empireId && row.forbiddenEmpireId == forbiddenEmpireId)
			{
				return true;
			}
		}

		return false;
	}

	void setCivilianHaulForbidden(
		WorldData& world,
		long long empireId,
		long long forbiddenEmpireId,
		bool forbidden)
	{
		if (empireId == 0 || forbiddenEmpireId == 0 || empireId == forbiddenEmpireId)
		{
			return;
		}

		if (!forbidden)
		{
			world.civilianHaulForbids.erase(
				std::remove_if(
					world.civilianHaulForbids.begin(),
					world.civilianHaulForbids.end(),
					[empireId, forbiddenEmpireId](const CivilianHaulForbid& row)
					{
						return row.empireId == empireId && row.forbiddenEmpireId == forbiddenEmpireId;
					}),
				world.civilianHaulForbids.end());
			return;
		}

		if (civilianHaulIsForbidden(world, empireId, forbiddenEmpireId))
		{
			return;
		}

		world.civilianHaulForbids.push_back(CivilianHaulForbid{
			.empireId = empireId,
			.forbiddenEmpireId = forbiddenEmpireId });
	}

}
