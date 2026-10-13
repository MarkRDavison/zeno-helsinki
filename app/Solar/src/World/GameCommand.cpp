#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <algorithm>
#include <type_traits>
#include <utility>

namespace slr
{
namespace
{

	bool applyMoveFleet(WorldData& world, const MoveFleetToSystem& order)
	{
		Fleet* fleet = world.fleetById(order.fleetId);
		if (fleet == nullptr || world.systemById(order.systemId) == nullptr)
		{
			return false;
		}

		if (fleet->motion.progress > 0.0f)
		{
			return false;
		}

		if (fleet->escortConvoyId != 0)
		{
			const Convoy* escort = world.convoyById(fleet->escortConvoyId);
			if (escort != nullptr && (escort->haulJobId != 0 || convoyIsUnderway(*escort)))
			{
				return false;
			}
		}

		if (fleet->systemId == order.systemId)
		{
			return false;
		}

		const std::vector<long long> path = shortestLanePath(world, fleet->systemId, order.systemId);
		if (path.size() < 2)
		{
			return false;
		}

		fleet->motion.route = path;
		fleet->motion.hop = 0;
		fleet->motion.progress = 0.0f;
		fleet->motion.direction = 1;
		return true;
	}

	bool applyAcceptHaulJob(WorldData& world, const AcceptHaulJob& order)
	{
		Truck* truck = world.truckById(order.truckId);
		HaulJob* job = world.haulJobById(order.jobId);
		if (truck == nullptr || job == nullptr)
		{
			return false;
		}

		if (truck->haulJobId != 0 || truck->cargoAmount != 0)
		{
			return false;
		}

		const Convoy* existingConvoy = world.convoyById(truck->convoyId);
		if (existingConvoy != nullptr && convoyIsStanding(*existingConvoy))
		{
			return false;
		}

		if (truck->empireId != job->empireId)
		{
			return false;
		}

		const StarSystem* destination = world.systemById(job->destinationSystemId);
		if (!truckHasCargoHaulRole(world, *truck))
		{
			return false;
		}

		if (!truckUsesMilitaryHaulPath(world, *truck) && destination != nullptr &&
			destination->empireId != 0 &&
			civilianHaulIsForbidden(world, truck->empireId, destination->empireId))
		{
			return false;
		}

		const long long truckload = truckloadForGood(job->goodId);
		if (job->amount < truckload)
		{
			return false;
		}

		const long long here = truckSystemId(*truck);
		if (here == 0)
		{
			return false;
		}

		const bool militaryPath = truckUsesMilitaryHaulPath(world, *truck);
		Convoy* convoy = openConvoyForHaulJob(world, job->id, militaryPath);
		long long loadSystemId = 0;
		long long assemblingConvoyId = 0;
		if (convoy != nullptr)
		{
			loadSystemId = convoy->loadSystemId;
			assemblingConvoyId = convoy->id;
		}
		else
		{
			loadSystemId = nearestSupplySystemId(
				world,
				here,
				job->goodId,
				job->destinationSystemId,
				truck->empireId,
				false,
				militaryPath);
		}

		if (loadSystemId == 0)
		{
			return false;
		}

		if (!haulWaveHasStockpileForAnotherTruck(world, loadSystemId, job->goodId, assemblingConvoyId))
		{
			return false;
		}

		if (convoy == nullptr)
		{
			world.convoys.push_back(Convoy{
				.id = nextConvoyId(world),
				.haulJobId = job->id,
				.empireId = job->empireId,
				.loadSystemId = loadSystemId });
			convoy = &world.convoys.back();
		}

		if (haulLanePathCost(
				world,
				loadSystemId,
				job->destinationSystemId,
				truck->empireId,
				false,
				militaryPath) < 0)
		{
			return false;
		}

		if (here != convoy->loadSystemId)
		{
			const std::vector<long long> path = haulLanePath(
				world,
				here,
				convoy->loadSystemId,
				truck->empireId,
				false,
				militaryPath);
			if (path.size() < 2)
			{
				return false;
			}

			truck->motion.route = path;
			truck->motion.hop = 0;
			truck->motion.progress = 0.0f;
			truck->motion.direction = 1;
		}
		else
		{
			parkTruckAt(*truck, convoy->loadSystemId);
		}

		job->amount -= truckload;
		truck->haulJobId = job->id;
		truck->convoyId = convoy->id;
		return true;
	}

	bool policyBelongsToEmpire(const WorldData& world, long long policyId, long long empireId)
	{
		const EngagementPolicy* policy = world.engagementPolicyById(policyId);
		return policy != nullptr && policy->empireId == empireId;
	}

	bool applyUpsertEngagementPolicy(WorldData& world, const UpsertEngagementPolicy& order)
	{
		if (world.empireById(order.empireId) == nullptr || order.name.empty())
		{
			return false;
		}

		if (order.policyId == 0)
		{
			long long nextId = 1;
			for (const auto& policy : world.engagementPolicies)
			{
				if (policy.id >= nextId)
				{
					nextId = policy.id + 1;
				}
			}

			world.engagementPolicies.push_back(EngagementPolicy{
				.id = nextId,
				.empireId = order.empireId,
				.name = order.name,
				.ruleOfEngagement = order.ruleOfEngagement });
			return true;
		}

		EngagementPolicy* existing = world.engagementPolicyById(order.policyId);
		if (existing == nullptr || existing->empireId != order.empireId)
		{
			return false;
		}

		existing->name = order.name;
		existing->ruleOfEngagement = order.ruleOfEngagement;
		return true;
	}

	bool applySetEmpireDefaultPolicy(WorldData& world, const SetEmpireDefaultPolicy& order)
	{
		Empire* empire = world.empireById(order.empireId);
		if (empire == nullptr || !policyBelongsToEmpire(world, order.policyId, order.empireId))
		{
			return false;
		}

		empire->defaultPolicyId = order.policyId;
		return true;
	}

	bool applySetFleetPolicyOverride(WorldData& world, const SetFleetPolicyOverride& order)
	{
		Fleet* fleet = world.fleetById(order.fleetId);
		if (fleet == nullptr)
		{
			return false;
		}

		if (order.policyId != 0 && !policyBelongsToEmpire(world, order.policyId, fleet->empireId))
		{
			return false;
		}

		fleet->policyId = order.policyId;
		return true;
	}

	void releaseTrucksFromConvoy(WorldData& world, long long convoyId)
	{
		for (auto& truck : world.trucks)
		{
			if (truck.convoyId == convoyId)
			{
				truck.convoyId = 0;
			}
		}
	}

	void removeConvoyById(WorldData& world, long long convoyId)
	{
		world.convoys.erase(
			std::remove_if(
				world.convoys.begin(),
				world.convoys.end(),
				[convoyId](const Convoy& convoy) { return convoy.id == convoyId; }),
			world.convoys.end());
	}

	long long countStandingKind(const WorldData& world, long long convoyId, HullRole role)
	{
		if (role == HullRole::MilitaryCargo)
		{
			return countMilitaryTrucksInConvoy(world, convoyId);
		}

		if (role == HullRole::Tender)
		{
			return countTendersInConvoy(world, convoyId);
		}

		return countCivilianTrucksInConvoy(world, convoyId);
	}

	bool dropEmptyStandingTrucksOfKind(
		WorldData& world,
		long long convoyId,
		HullRole role,
		long long keep)
	{
		while (countStandingKind(world, convoyId, role) > keep)
		{
			Truck* extra = nullptr;
			for (auto& truck : world.trucks)
			{
				if (truck.convoyId != convoyId || truck.haulJobId != 0 || truck.cargoAmount != 0)
				{
					continue;
				}

				if (!truckHasRole(world, truck, role))
				{
					continue;
				}

				if (extra == nullptr || truck.id > extra->id)
				{
					extra = &truck;
				}
			}

			if (extra == nullptr)
			{
				return false;
			}

			extra->convoyId = 0;
		}

		return true;
	}

	bool applySetFleetEscort(WorldData& world, const SetFleetEscort& order)
	{
		Fleet* fleet = world.fleetById(order.fleetId);
		if (fleet == nullptr || order.desiredCivilianTruckCount < 0 ||
			order.desiredMilitaryTruckCount < 0 || order.desiredTenderCount < 0)
		{
			return false;
		}

		if (!order.escorting)
		{
			if (fleet->escortConvoyId == 0)
			{
				return false;
			}

			Convoy* convoy = world.convoyById(fleet->escortConvoyId);
			fleet->escortConvoyId = 0;
			if (convoy == nullptr)
			{
				return true;
			}

			if (convoy->haulJobId == 0)
			{
				const long long convoyId = convoy->id;
				releaseTrucksFromConvoy(world, convoyId);
				removeConvoyById(world, convoyId);
			}
			else
			{
				convoy->escortFleetId = 0;
				convoy->desiredCivilianTruckCount = 0;
				convoy->desiredMilitaryTruckCount = 0;
				convoy->desiredTenderCount = 0;
			}

			return true;
		}

		if (fleet->escortConvoyId != 0)
		{
			Convoy* convoy = world.convoyById(fleet->escortConvoyId);
			if (convoy == nullptr || !convoyIsStanding(*convoy))
			{
				return false;
			}

			if (order.desiredCivilianTruckCount < convoy->desiredCivilianTruckCount &&
				!dropEmptyStandingTrucksOfKind(
					world,
					convoy->id,
					HullRole::CivilianCargo,
					order.desiredCivilianTruckCount))
			{
				return false;
			}

			if (order.desiredMilitaryTruckCount < convoy->desiredMilitaryTruckCount &&
				!dropEmptyStandingTrucksOfKind(
					world,
					convoy->id,
					HullRole::MilitaryCargo,
					order.desiredMilitaryTruckCount))
			{
				return false;
			}

			if (order.desiredTenderCount < convoy->desiredTenderCount &&
				!dropEmptyStandingTrucksOfKind(
					world,
					convoy->id,
					HullRole::Tender,
					order.desiredTenderCount))
			{
				return false;
			}

			convoy->desiredCivilianTruckCount = order.desiredCivilianTruckCount;
			convoy->desiredMilitaryTruckCount = order.desiredMilitaryTruckCount;
			convoy->desiredTenderCount = order.desiredTenderCount;
			return true;
		}

		world.convoys.push_back(Convoy{
			.id = nextConvoyId(world),
			.empireId = fleet->empireId,
			.loadSystemId = fleetRallySystemId(*fleet),
			.escortFleetId = fleet->id,
			.desiredCivilianTruckCount = order.desiredCivilianTruckCount,
			.desiredMilitaryTruckCount = order.desiredMilitaryTruckCount,
			.desiredTenderCount = order.desiredTenderCount });
		fleet->escortConvoyId = world.convoys.back().id;
		return true;
	}

	bool applyJoinStandingConvoy(WorldData& world, const JoinStandingConvoy& order)
	{
		Truck* truck = world.truckById(order.truckId);
		Convoy* convoy = world.convoyById(order.convoyId);
		if (truck == nullptr || convoy == nullptr)
		{
			return false;
		}

		if (!convoyIsStanding(*convoy) || convoy->empireId != truck->empireId)
		{
			return false;
		}

		if (truck->haulJobId != 0 || truck->cargoAmount != 0 || truck->convoyId != 0)
		{
			return false;
		}

		if (!standingConvoyHasJoinSlotForTruck(world, *convoy, *truck))
		{
			return false;
		}

		const Fleet* fleet = world.fleetById(convoy->escortFleetId);
		if (fleet == nullptr)
		{
			return false;
		}

		const long long rally = fleetRallySystemId(*fleet);
		if (rally == 0)
		{
			return false;
		}

		truck->convoyId = convoy->id;
		if (!pathTruckToSystem(world, *truck, rally))
		{
			truck->convoyId = 0;
			return false;
		}

		return true;
	}

	bool standingConvoyMembersReadyAtRally(const WorldData& world, const Convoy& convoy)
	{
		if (!convoyIsStanding(convoy) || standingDesiredTruckCount(convoy) <= 0 ||
			convoy.haulJobId != 0)
		{
			return false;
		}

		if (countCargoHaulTrucksInConvoy(world, convoy.id) != standingDesiredTruckCount(convoy))
		{
			return false;
		}

		const Fleet* fleet = world.fleetById(convoy.escortFleetId);
		if (fleet == nullptr || fleet->motion.route.size() >= 2)
		{
			return false;
		}

		const long long rally = fleetRallySystemId(*fleet);
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId != convoy.id || !truckHasCargoHaulRole(world, truck))
			{
				continue;
			}

			if (truck.haulJobId != 0 || truck.cargoAmount != 0 || !isTruckAtSystem(truck, rally))
			{
				return false;
			}
		}

		return true;
	}

	bool applyAcceptStandingConvoyHaulJob(WorldData& world, const AcceptStandingConvoyHaulJob& order)
	{
		Convoy* convoy = world.convoyById(order.convoyId);
		HaulJob* job = world.haulJobById(order.jobId);
		if (convoy == nullptr || job == nullptr)
		{
			return false;
		}

		if (!standingConvoyMembersReadyAtRally(world, *convoy))
		{
			return false;
		}

		if (job->empireId != convoy->empireId)
		{
			return false;
		}

		const long long truckload = truckloadForGood(job->goodId);
		const long long reserved = standingDesiredTruckCount(*convoy) * truckload;
		if (job->amount < reserved)
		{
			return false;
		}

		const Fleet* fleet = world.fleetById(convoy->escortFleetId);
		if (fleet == nullptr)
		{
			return false;
		}

		const long long rally = fleetRallySystemId(*fleet);
		const long long loadSystemId =
			nearestSupplySystemId(
				world,
				rally,
				job->goodId,
				job->destinationSystemId,
				convoy->empireId,
				true,
				standingConvoyUsesMilitaryHaulPath(*convoy));
		if (loadSystemId == 0)
		{
			return false;
		}

		const StarSystem* supply = world.systemById(loadSystemId);
		if (supply == nullptr || getStockpileAmount(*supply, job->goodId) < reserved)
		{
			return false;
		}

		job->amount -= reserved;
		convoy->haulJobId = job->id;
		convoy->loadSystemId = loadSystemId;
		for (auto& truck : world.trucks)
		{
			if (truck.convoyId != convoy->id || !truckHasCargoHaulRole(world, truck))
			{
				continue;
			}

			truck.haulJobId = job->id;
		}

		return true;
	}

	bool applySetCivilianHaulForbidden(WorldData& world, const SetCivilianHaulForbidden& order)
	{
		if (world.empireById(order.empireId) == nullptr ||
			world.empireById(order.forbiddenEmpireId) == nullptr ||
			order.empireId == order.forbiddenEmpireId)
		{
			return false;
		}

		setCivilianHaulForbidden(world, order.empireId, order.forbiddenEmpireId, order.forbidden);
		return true;
	}

	bool applyStartYardSlip(WorldData& world, const StartYardSlip& order)
	{
		Yard* yard = yardAtSystem(world, order.systemId);
		if (yard == nullptr)
		{
			return false;
		}

		if (stockpileAvailableForYardConsume(world, order.systemId, SteelGoodId) < YardSlipSteelCost)
		{
			return false;
		}

		if (!useFromStockpile(world, order.systemId, SteelGoodId, YardSlipSteelCost))
		{
			return false;
		}

		yard->slips.push_back(YardSlip{
			.id = nextYardSlipId(world),
			.remainingBuildTicks = YardSlipBuildTicks });
		return true;
	}

	bool applyStartYardBuild(WorldData& world, const StartYardBuild& order)
	{
		Yard* yard = yardAtSystem(world, order.systemId);
		if (yard == nullptr)
		{
			return false;
		}

		const ShipClass* shipClass = world.shipClassById(order.classId);
		if (shipClass == nullptr || !shipClassIsTruckOfEmpire(*shipClass, yard->empireId))
		{
			return false;
		}

		if (shipClass->steelCost <= 0 || shipClass->buildTicks <= 0)
		{
			return false;
		}

		yard->hullBuildQueue.push_back(shipClass->id);
		return true;
	}

	bool applyStartYardRefit(WorldData& world, const StartYardRefit& order)
	{
		if (order.fromClassId == order.toClassId)
		{
			return false;
		}

		Yard* yard = yardAtSystem(world, order.systemId);
		if (yard == nullptr)
		{
			return false;
		}

		const ShipClass* fromClass = world.shipClassById(order.fromClassId);
		const ShipClass* toClass = world.shipClassById(order.toClassId);
		if (fromClass == nullptr || toClass == nullptr)
		{
			return false;
		}

		if (!shipClassIsWarshipOfEmpire(*fromClass, yard->empireId) ||
			!shipClassIsWarshipOfEmpire(*toClass, yard->empireId))
		{
			return false;
		}

		if (fromClass->hullSize != toClass->hullSize)
		{
			return false;
		}

		Fleet* sourceFleet = nullptr;
		long long hullId = 0;
		for (auto& fleet : world.fleets)
		{
			if (fleet.empireId != yard->empireId || fleet.systemId != yard->systemId ||
				!fleetIsParked(fleet))
			{
				continue;
			}

			long long bestHullId = 0;
			for (const auto& hull : fleet.hulls)
			{
				if (hull.classId != order.fromClassId)
				{
					continue;
				}

				if (bestHullId == 0 || hull.id < bestHullId)
				{
					bestHullId = hull.id;
				}
			}

			if (bestHullId == 0)
			{
				continue;
			}

			if (sourceFleet == nullptr || fleet.id < sourceFleet->id)
			{
				sourceFleet = &fleet;
				hullId = bestHullId;
			}
		}

		if (sourceFleet == nullptr || hullId == 0)
		{
			return false;
		}

		if (!removeFleetHull(*sourceFleet, hullId))
		{
			return false;
		}

		yard->hullRefitQueue.push_back(YardRefitJob{
			.hullId = hullId,
			.fromClassId = order.fromClassId,
			.toClassId = order.toClassId,
			.preferredFleetId = sourceFleet->id });
		return true;
	}

}

	GameCommand GameCommand::moveFleetToSystem(
		long long fleetId,
		long long systemId,
		CommandSource source)
	{
		GameCommand command;
		command.source = source;
		command.payload = MoveFleetToSystem{ .fleetId = fleetId, .systemId = systemId };
		return command;
	}

	GameCommand GameCommand::acceptHaulJob(
		long long truckId,
		long long jobId,
		CommandSource source)
	{
		GameCommand command;
		command.source = source;
		command.payload = AcceptHaulJob{ .truckId = truckId, .jobId = jobId };
		return command;
	}

	GameCommand GameCommand::upsertEngagementPolicy(
		long long policyId,
		long long empireId,
		std::string name,
		RuleOfEngagement ruleOfEngagement,
		CommandSource source)
	{
		GameCommand command;
		command.source = source;
		command.payload = UpsertEngagementPolicy{
			.policyId = policyId,
			.empireId = empireId,
			.name = std::move(name),
			.ruleOfEngagement = ruleOfEngagement };
		return command;
	}

	GameCommand GameCommand::setEmpireDefaultPolicy(
		long long empireId,
		long long policyId,
		CommandSource source)
	{
		GameCommand command;
		command.source = source;
		command.payload = SetEmpireDefaultPolicy{ .empireId = empireId, .policyId = policyId };
		return command;
	}

	GameCommand GameCommand::setFleetPolicyOverride(
		long long fleetId,
		long long policyId,
		CommandSource source)
	{
		GameCommand command;
		command.source = source;
		command.payload = SetFleetPolicyOverride{ .fleetId = fleetId, .policyId = policyId };
		return command;
	}

	GameCommand GameCommand::setFleetEscort(
		long long fleetId,
		bool escorting,
		long long desiredCivilianTruckCount,
		long long desiredMilitaryTruckCount,
		long long desiredTenderCount,
		CommandSource source)
	{
		GameCommand command;
		command.source = source;
		command.payload = SetFleetEscort{
			.fleetId = fleetId,
			.escorting = escorting,
			.desiredCivilianTruckCount = desiredCivilianTruckCount,
			.desiredMilitaryTruckCount = desiredMilitaryTruckCount,
			.desiredTenderCount = desiredTenderCount };
		return command;
	}

	GameCommand GameCommand::joinStandingConvoy(
		long long truckId,
		long long convoyId,
		CommandSource source)
	{
		GameCommand command;
		command.source = source;
		command.payload = JoinStandingConvoy{ .truckId = truckId, .convoyId = convoyId };
		return command;
	}

	GameCommand GameCommand::acceptStandingConvoyHaulJob(
		long long convoyId,
		long long jobId,
		CommandSource source)
	{
		GameCommand command;
		command.source = source;
		command.payload = AcceptStandingConvoyHaulJob{ .convoyId = convoyId, .jobId = jobId };
		return command;
	}

	GameCommand GameCommand::setCivilianHaulForbidden(
		long long empireId,
		long long forbiddenEmpireId,
		bool forbidden,
		CommandSource source)
	{
		GameCommand command;
		command.source = source;
		command.payload = SetCivilianHaulForbidden{
			.empireId = empireId,
			.forbiddenEmpireId = forbiddenEmpireId,
			.forbidden = forbidden };
		return command;
	}

	GameCommand GameCommand::startYardSlip(long long systemId, CommandSource source)
	{
		GameCommand command;
		command.source = source;
		command.payload = StartYardSlip{ .systemId = systemId };
		return command;
	}

	GameCommand GameCommand::startYardBuild(long long systemId, long long classId, CommandSource source)
	{
		GameCommand command;
		command.source = source;
		command.payload = StartYardBuild{ .systemId = systemId, .classId = classId };
		return command;
	}

	GameCommand GameCommand::startYardRefit(
		long long systemId,
		long long fromClassId,
		long long toClassId,
		CommandSource source)
	{
		GameCommand command;
		command.source = source;
		command.payload = StartYardRefit{
			.systemId = systemId,
			.fromClassId = fromClassId,
			.toClassId = toClassId };
		return command;
	}

	bool execute(WorldData& world, const GameCommand& command)
	{
		return std::visit(
			[&world](const auto& payload)
			{
				using Payload = std::decay_t<decltype(payload)>;
				if constexpr (std::is_same_v<Payload, MoveFleetToSystem>)
				{
					return applyMoveFleet(world, payload);
				}
				else if constexpr (std::is_same_v<Payload, AcceptHaulJob>)
				{
					return applyAcceptHaulJob(world, payload);
				}
				else if constexpr (std::is_same_v<Payload, UpsertEngagementPolicy>)
				{
					return applyUpsertEngagementPolicy(world, payload);
				}
				else if constexpr (std::is_same_v<Payload, SetEmpireDefaultPolicy>)
				{
					return applySetEmpireDefaultPolicy(world, payload);
				}
				else if constexpr (std::is_same_v<Payload, SetFleetPolicyOverride>)
				{
					return applySetFleetPolicyOverride(world, payload);
				}
				else if constexpr (std::is_same_v<Payload, SetFleetEscort>)
				{
					return applySetFleetEscort(world, payload);
				}
				else if constexpr (std::is_same_v<Payload, JoinStandingConvoy>)
				{
					return applyJoinStandingConvoy(world, payload);
				}
				else if constexpr (std::is_same_v<Payload, AcceptStandingConvoyHaulJob>)
				{
					return applyAcceptStandingConvoyHaulJob(world, payload);
				}
				else if constexpr (std::is_same_v<Payload, SetCivilianHaulForbidden>)
				{
					return applySetCivilianHaulForbidden(world, payload);
				}
				else if constexpr (std::is_same_v<Payload, StartYardSlip>)
				{
					return applyStartYardSlip(world, payload);
				}
				else if constexpr (std::is_same_v<Payload, StartYardBuild>)
				{
					return applyStartYardBuild(world, payload);
				}
				else if constexpr (std::is_same_v<Payload, StartYardRefit>)
				{
					return applyStartYardRefit(world, payload);
				}
				else
				{
					return false;
				}
			},
			command.payload);
	}

}
