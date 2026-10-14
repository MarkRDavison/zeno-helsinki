#include <World/Intel.hpp>
#include <algorithm>
#include <set>
#include <vector>

namespace slr
{
namespace
{

	long long hullCountInComposition(const std::vector<ClassCount>& composition)
	{
		long long count = 0;
		for (const auto& row : composition)
		{
			count += row.count;
		}

		return count;
	}

	long long survivorHullCountForEmpire(const BattleReport& report, long long empireId)
	{
		long long count = 0;
		for (const auto& side : report.sides)
		{
			if (side.empireId == empireId)
			{
				count += hullCountInComposition(side.after);
			}
		}

		return count;
	}

	std::vector<long long> empiresInReport(const BattleReport& report)
	{
		std::set<long long> empires;
		for (const auto& side : report.sides)
		{
			empires.insert(side.empireId);
		}

		return std::vector<long long>(empires.begin(), empires.end());
	}

	std::vector<long long> subjectClassIdsInReport(
		const BattleReport& report,
		long long subjectEmpireId)
	{
		std::set<long long> classIds;
		for (const auto& side : report.sides)
		{
			if (side.empireId != subjectEmpireId)
			{
				continue;
			}

			for (const auto& row : side.before)
			{
				if (row.count > 0)
				{
					classIds.insert(row.classId);
				}
			}
		}

		return std::vector<long long>(classIds.begin(), classIds.end());
	}

	ObservedEnemyShipClass* findObservation(
		WorldData& world,
		long long observerEmpireId,
		long long subjectEmpireId,
		long long classId)
	{
		for (auto& row : world.observedEnemyShipClasses)
		{
			if (row.observerEmpireId == observerEmpireId && row.subjectEmpireId == subjectEmpireId &&
				row.classId == classId)
			{
				return &row;
			}
		}

		return nullptr;
	}

	void recordEnemyClassSighting(
		WorldData& world,
		BattleReport& report,
		long long observerEmpireId,
		long long subjectEmpireId,
		long long classId)
	{
		ObservedEnemyShipClass* existing =
			findObservation(world, observerEmpireId, subjectEmpireId, classId);
		if (existing != nullptr)
		{
			++existing->sightingCount;
			return;
		}

		world.observedEnemyShipClasses.push_back(ObservedEnemyShipClass{
			.observerEmpireId = observerEmpireId,
			.subjectEmpireId = subjectEmpireId,
			.classId = classId,
			.firstObservedTick = world.tick,
			.sightingCount = 1 });
		report.intelGainedThisBattle.push_back(BattleIntelGain{
			.observerEmpireId = observerEmpireId,
			.subjectEmpireId = subjectEmpireId,
			.classId = classId });
	}

}

	const ObservedEnemyShipClass* observedEnemyShipClass(
		const WorldData& world,
		long long observerEmpireId,
		long long subjectEmpireId,
		long long classId)
	{
		for (const auto& row : world.observedEnemyShipClasses)
		{
			if (row.observerEmpireId == observerEmpireId && row.subjectEmpireId == subjectEmpireId &&
				row.classId == classId)
			{
				return &row;
			}
		}

		return nullptr;
	}

	bool observerHasSeenEnemyClass(
		const WorldData& world,
		long long observerEmpireId,
		long long subjectEmpireId,
		long long classId)
	{
		return observedEnemyShipClass(world, observerEmpireId, subjectEmpireId, classId) != nullptr;
	}

	long long countObservedEnemyShipClassesForObserver(
		const WorldData& world,
		long long observerEmpireId)
	{
		long long count = 0;
		for (const auto& row : world.observedEnemyShipClasses)
		{
			if (row.observerEmpireId == observerEmpireId)
			{
				++count;
			}
		}

		return count;
	}

	void applyBattleSurvivorIntel(WorldData& world, BattleReport& report)
	{
		const std::vector<long long> empires = empiresInReport(report);
		for (const long long observerEmpireId : empires)
		{
			if (survivorHullCountForEmpire(report, observerEmpireId) <= 0)
			{
				continue;
			}

			for (const long long subjectEmpireId : empires)
			{
				if (subjectEmpireId == observerEmpireId)
				{
					continue;
				}

				const std::vector<long long> classIds = subjectClassIdsInReport(report, subjectEmpireId);
				for (const long long classId : classIds)
				{
					recordEnemyClassSighting(world, report, observerEmpireId, subjectEmpireId, classId);
				}
			}
		}
	}

}