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

		Convoy* convoy = openConvoyForHaulJob(world, job->id);
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
				job->destinationSystemId);
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

		if (here != convoy->loadSystemId)
		{
			const std::vector<long long> path = shortestLanePath(world, here, convoy->loadSystemId);
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

	bool applySetFleetEscort(WorldData& world, const SetFleetEscort& order)
	{
		Fleet* fleet = world.fleetById(order.fleetId);
		if (fleet == nullptr || order.desiredTruckCount < 0)
		{
			return false;
		}

		if (order.desiredTruckCount == 0)
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
				convoy->desiredTruckCount = 0;
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

			if (order.desiredTruckCount < convoy->desiredTruckCount)
			{
				while (countTrucksInConvoy(world, convoy->id) > order.desiredTruckCount)
				{
					Truck* extra = nullptr;
					for (auto& truck : world.trucks)
					{
						if (truck.convoyId != convoy->id || truck.haulJobId != 0 || truck.cargoAmount != 0)
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
			}

			convoy->desiredTruckCount = order.desiredTruckCount;
			return true;
		}

		world.convoys.push_back(Convoy{
			.id = nextConvoyId(world),
			.empireId = fleet->empireId,
			.loadSystemId = fleetRallySystemId(*fleet),
			.escortFleetId = fleet->id,
			.desiredTruckCount = order.desiredTruckCount });
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

		if (countTrucksInConvoy(world, convoy->id) >= convoy->desiredTruckCount)
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

		pathTruckToSystem(world, *truck, rally);
		truck->convoyId = convoy->id;
		return true;
	}

	bool standingConvoyMembersReadyAtRally(const WorldData& world, const Convoy& convoy)
	{
		if (!convoyIsStanding(convoy) || convoy.desiredTruckCount <= 0 || convoy.haulJobId != 0)
		{
			return false;
		}

		if (countTrucksInConvoy(world, convoy.id) != convoy.desiredTruckCount)
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
			if (truck.convoyId != convoy.id)
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
		const long long reserved = convoy->desiredTruckCount * truckload;
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
			nearestSupplySystemId(world, rally, job->goodId, job->destinationSystemId);
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
			if (truck.convoyId != convoy->id)
			{
				continue;
			}

			truck.haulJobId = job->id;
		}

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
		long long desiredTruckCount,
		CommandSource source)
	{
		GameCommand command;
		command.source = source;
		command.payload = SetFleetEscort{ .fleetId = fleetId, .desiredTruckCount = desiredTruckCount };
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
				else
				{
					return false;
				}
			},
			command.payload);
	}

}
