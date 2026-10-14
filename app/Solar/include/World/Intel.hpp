#pragma once

#include <World/WorldData.hpp>
#include <string>

namespace slr
{

	constexpr int IntelKnownClassSightingThreshold = 4;

	enum class IntelConfidence
	{
		Unknown,
		Estimate,
		Known
	};

	struct EnemyClassIntelLine
	{
		std::string label;
		IntelConfidence confidence{ IntelConfidence::Unknown };
	};

	void applyBattleSurvivorIntel(WorldData& world, BattleReport& report);

	const ObservedEnemyShipClass* observedEnemyShipClass(
		const WorldData& world,
		long long observerEmpireId,
		long long subjectEmpireId,
		long long classId);

	bool observerHasSeenEnemyClass(
		const WorldData& world,
		long long observerEmpireId,
		long long subjectEmpireId,
		long long classId);

	long long countObservedEnemyShipClassesForObserver(
		const WorldData& world,
		long long observerEmpireId);

	IntelConfidence enemyClassIdentityConfidence(
		const WorldData& world,
		long long observerEmpireId,
		long long subjectEmpireId,
		long long classId);

	bool shipClassesAreIntelSimilar(const ShipClass& a, const ShipClass& b);

	std::vector<long long> similarObservedEnemyClasses(
		const WorldData& world,
		long long observerEmpireId,
		long long subjectEmpireId,
		long long classId);

	EnemyClassIntelLine formatEnemyClassIntelLine(
		const WorldData& world,
		long long observerEmpireId,
		long long subjectEmpireId,
		long long classId,
		long long count);

	std::string formatBattleIntelGainLine(
		const WorldData& world,
		long long observerEmpireId,
		const BattleIntelGain& gain);

}