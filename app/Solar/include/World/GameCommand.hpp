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

	struct SetFleetEscort
	{
		long long fleetId{ 0 };
		long long desiredCivilianTruckCount{ 0 };
		long long desiredMilitaryTruckCount{ 0 };
	};

	struct JoinStandingConvoy
	{
		long long truckId{ 0 };
		long long convoyId{ 0 };
	};

	struct AcceptStandingConvoyHaulJob
	{
		long long convoyId{ 0 };
		long long jobId{ 0 };
	};

	struct SetCivilianHaulForbidden
	{
		long long empireId{ 0 };
		long long forbiddenEmpireId{ 0 };
		bool forbidden{ false };
	};

	struct StartYardSlip
	{
		long long systemId{ 0 };
	};

	struct GameCommand
	{
		CommandSource source{ CommandSource::Player };
		std::variant<
			MoveFleetToSystem,
			AcceptHaulJob,
			UpsertEngagementPolicy,
			SetEmpireDefaultPolicy,
			SetFleetPolicyOverride,
			SetFleetEscort,
			JoinStandingConvoy,
			AcceptStandingConvoyHaulJob,
			SetCivilianHaulForbidden,
			StartYardSlip>
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
		static GameCommand setFleetEscort(
			long long fleetId,
			long long desiredCivilianTruckCount,
			long long desiredMilitaryTruckCount,
			CommandSource source);
		static GameCommand joinStandingConvoy(
			long long truckId,
			long long convoyId,
			CommandSource source);
		static GameCommand acceptStandingConvoyHaulJob(
			long long convoyId,
			long long jobId,
			CommandSource source);
		static GameCommand setCivilianHaulForbidden(
			long long empireId,
			long long forbiddenEmpireId,
			bool forbidden,
			CommandSource source);
		static GameCommand startYardSlip(long long systemId, CommandSource source);
	};

	bool execute(WorldData& world, const GameCommand& command);

}
