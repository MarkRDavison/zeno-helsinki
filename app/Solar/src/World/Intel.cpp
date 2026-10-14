#include <World/Intel.hpp>
#include <World/ShipDesign.hpp>
#include <algorithm>
#include <cmath>
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

namespace
{

	std::string lowerHullSizeLabel(HullSize size)
	{
		if (size == HullSize::Medium)
		{
			return "medium";
		}

		if (size == HullSize::Large)
		{
			return "large";
		}

		return "small";
	}

	int classSightingCount(
		const WorldData& world,
		long long observerEmpireId,
		long long subjectEmpireId,
		long long classId)
	{
		const ObservedEnemyShipClass* row =
			observedEnemyShipClass(world, observerEmpireId, subjectEmpireId, classId);
		return row != nullptr ? row->sightingCount : 0;
	}

	bool laserDamageWithinIntelBand(long long a, long long b)
	{
		if (a == b)
		{
			return true;
		}

		const long long hi = std::max(a, b);
		const long long lo = std::min(a, b);
		if (hi == 0)
		{
			return true;
		}

		return static_cast<double>(lo) >= static_cast<double>(hi) * 0.75;
	}

	bool classShowsDisambiguatedName(
		const WorldData& world,
		long long observerEmpireId,
		long long subjectEmpireId,
		long long classId)
	{
		const int sightings = classSightingCount(world, observerEmpireId, subjectEmpireId, classId);
		if (sightings >= IntelKnownClassSightingThreshold)
		{
			return true;
		}

		if (sightings < 2)
		{
			return false;
		}

		const std::vector<long long> similar =
			similarObservedEnemyClasses(world, observerEmpireId, subjectEmpireId, classId);
		for (const long long peerClassId : similar)
		{
			if (classSightingCount(world, observerEmpireId, subjectEmpireId, peerClassId) >= sightings)
			{
				return false;
			}
		}

		return true;
	}

	std::string identityLabelForClass(
		const WorldData& world,
		long long observerEmpireId,
		long long subjectEmpireId,
		long long classId)
	{
		const ShipClass* shipClass = world.shipClassById(classId);
		if (shipClass == nullptr)
		{
			return "Unknown warship";
		}

		const IntelConfidence confidence =
			enemyClassIdentityConfidence(world, observerEmpireId, subjectEmpireId, classId);
		if (confidence == IntelConfidence::Unknown)
		{
			return "Unknown warship";
		}

		if (confidence == IntelConfidence::Known)
		{
			return shipClass->name;
		}

		const int sightings = classSightingCount(world, observerEmpireId, subjectEmpireId, classId);
		const std::string sizeLabel = lowerHullSizeLabel(shipClass->hullSize);
		if (sightings == 1)
		{
			return "New " + sizeLabel + " hull detected";
		}

		if (classShowsDisambiguatedName(world, observerEmpireId, subjectEmpireId, classId))
		{
			return shipClass->name + " (estimate)";
		}

		const std::vector<long long> similar =
			similarObservedEnemyClasses(world, observerEmpireId, subjectEmpireId, classId);
		for (const long long peerClassId : similar)
		{
			const int peerSightings =
				classSightingCount(world, observerEmpireId, subjectEmpireId, peerClassId);
			if (peerSightings < sightings)
			{
				continue;
			}

			if (classShowsDisambiguatedName(world, observerEmpireId, subjectEmpireId, peerClassId))
			{
				const ShipClass* peerClass = world.shipClassById(peerClassId);
				if (peerClass != nullptr)
				{
					return hullSizeName(shipClass->hullSize) + std::string(" hull (may match ") +
						peerClass->name + ")";
				}
			}
		}

		if (!similar.empty())
		{
			return hullSizeName(shipClass->hullSize) + std::string(" hull — new or variant?");
		}

		return hullSizeName(shipClass->hullSize) + std::string(" hull (seen before)");
	}

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

	IntelConfidence enemyClassIdentityConfidence(
		const WorldData& world,
		long long observerEmpireId,
		long long subjectEmpireId,
		long long classId)
	{
		const int sightings = classSightingCount(world, observerEmpireId, subjectEmpireId, classId);
		if (sightings <= 0)
		{
			return IntelConfidence::Unknown;
		}

		if (sightings >= IntelKnownClassSightingThreshold)
		{
			return IntelConfidence::Known;
		}

		return IntelConfidence::Estimate;
	}

	bool shipClassesAreIntelSimilar(const ShipClass& a, const ShipClass& b)
	{
		if (a.hullSize != b.hullSize)
		{
			return false;
		}

		const bool aMissile = a.missileTubes > 0;
		const bool bMissile = b.missileTubes > 0;
		if (aMissile != bMissile)
		{
			return false;
		}

		return laserDamageWithinIntelBand(a.laserDamage, b.laserDamage);
	}

	std::vector<long long> similarObservedEnemyClasses(
		const WorldData& world,
		long long observerEmpireId,
		long long subjectEmpireId,
		long long classId)
	{
		const ShipClass* shipClass = world.shipClassById(classId);
		if (shipClass == nullptr)
		{
			return {};
		}

		std::set<long long> similar;
		for (const auto& row : world.observedEnemyShipClasses)
		{
			if (row.observerEmpireId != observerEmpireId || row.subjectEmpireId != subjectEmpireId ||
				row.classId == classId)
			{
				continue;
			}

			const ShipClass* other = world.shipClassById(row.classId);
			if (other != nullptr && shipClassesAreIntelSimilar(*shipClass, *other))
			{
				similar.insert(row.classId);
			}
		}

		return std::vector<long long>(similar.begin(), similar.end());
	}

	EnemyClassIntelLine formatEnemyClassIntelLine(
		const WorldData& world,
		long long observerEmpireId,
		long long subjectEmpireId,
		long long classId,
		long long count)
	{
		EnemyClassIntelLine line;
		line.confidence =
			enemyClassIdentityConfidence(world, observerEmpireId, subjectEmpireId, classId);
		line.label = identityLabelForClass(world, observerEmpireId, subjectEmpireId, classId);
		if (count > 0)
		{
			line.label += " " + std::to_string(count);
		}

		return line;
	}

	std::string formatBattleIntelGainLine(
		const WorldData& world,
		long long observerEmpireId,
		const BattleIntelGain& gain)
	{
		const std::string label =
			identityLabelForClass(world, observerEmpireId, gain.subjectEmpireId, gain.classId);
		return "Survivors reported: " + label;
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