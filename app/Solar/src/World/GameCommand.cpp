#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <type_traits>

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
		if (convoy == nullptr)
		{
			const long long supplyId = nearestSupplySystemId(
				world,
				here,
				job->goodId,
				job->destinationSystemId);
			if (supplyId == 0)
			{
				return false;
			}

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
				.loadSystemId = supplyId });
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
			},
			command.payload);
	}

}
