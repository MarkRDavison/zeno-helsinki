#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
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
			long long nextId = 1;
			for (const auto& existing : world.convoys)
			{
				if (existing.id >= nextId)
				{
					nextId = existing.id + 1;
				}
			}

			world.convoys.push_back(Convoy{
				.id = nextId,
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
				else
				{
					return false;
				}
			},
			command.payload);
	}

}
