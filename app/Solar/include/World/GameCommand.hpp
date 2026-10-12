#pragma once

#include <World/WorldData.hpp>
#include <string>
#include <variant>

namespace slr
{

	enum class CommandSource
	{
		Player,
		Ai
	};

	struct MoveFleetToSystem
	{
		long long fleetId{ 0 };
		long long systemId{ 0 };
	};

	struct AcceptHaulJob
	{
		long long truckId{ 0 };
		long long jobId{ 0 };
	};

	struct UpsertEngagementPolicy
	{
		long long policyId{ 0 };
		long long empireId{ 0 };
		std::string name;
		RuleOfEngagement ruleOfEngagement{ RuleOfEngagement::DoNotFireUnlessFiredUpon };
	};

	struct SetEmpireDefaultPolicy
	{
		long long empireId{ 0 };
		long long policyId{ 0 };
	};

	struct SetFleetPolicyOverride
	{
		long long fleetId{ 0 };
		long long policyId{ 0 };
	};

	struct GameCommand
	{
		CommandSource source{ CommandSource::Player };
		std::variant<
			MoveFleetToSystem,
			AcceptHaulJob,
			UpsertEngagementPolicy,
			SetEmpireDefaultPolicy,
			SetFleetPolicyOverride>
			payload;

		static GameCommand moveFleetToSystem(
			long long fleetId,
			long long systemId,
			CommandSource source);
		static GameCommand acceptHaulJob(
			long long truckId,
			long long jobId,
			CommandSource source);
		static GameCommand upsertEngagementPolicy(
			long long policyId,
			long long empireId,
			std::string name,
			RuleOfEngagement ruleOfEngagement,
			CommandSource source);
		static GameCommand setEmpireDefaultPolicy(
			long long empireId,
			long long policyId,
			CommandSource source);
		static GameCommand setFleetPolicyOverride(
			long long fleetId,
			long long policyId,
			CommandSource source);
	};

	bool execute(WorldData& world, const GameCommand& command);

}
