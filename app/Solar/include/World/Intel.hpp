#pragma once

#include <World/WorldData.hpp>

namespace slr
{

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

}