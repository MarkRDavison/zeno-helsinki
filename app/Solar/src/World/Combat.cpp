#include <World/Combat.hpp>
#include <World/Intel.hpp>
#include <World/TruckCargo.hpp>
#include <helsinki/System/Utils/SplitMix64.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <limits>
#include <map>
#include <numeric>
#include <set>
#include <vector>

namespace slr
{
namespace
{

	struct CombatShip
	{
		long long id{ 0 };
		long long fleetId{ 0 };
		long long empireId{ 0 };
		long long classId{ 0 };
		long long hullId{ 0 };
		float position{ 0.0f };
		float range{ 0.0f };
		float missileRange{ 0.0f };
		float accuracy{ 0.0f };
		long long laserDamage{ 0 };
		long long missileDamage{ 0 };
		long long missileTubes{ 0 };
		long long missileReloadInnerTicks{ 0 };
		long long missileReloadRemaining{ 0 };
		std::vector<GoodQuantity> magazine;
		float missileSpeed{ 0.0f };
		long long shields{ 0 };
		long long hull{ 0 };
		float closeRate{ 0.0f };
		FleetApproach approach{ FleetApproach::CloseToLaserRange };
	};

	struct CombatMissile
	{
		float position{ 0.0f };
		float speed{ 0.0f };
		float accuracy{ 0.0f };
		long long damage{ 0 };
		long long targetId{ 0 };
	};

	bool isParked(const Fleet& fleet)
	{
		return !(fleet.motion.progress > 0.0f);
	}

	bool living(const CombatShip& ship)
	{
		return ship.hull > 0;
	}

	const CombatShip* lowestLivingEnemyInWeaponRange(
		const std::vector<CombatShip>& ships,
		const CombatShip& self,
		float weaponRange)
	{
		const CombatShip* best = nullptr;
		for (const auto& ship : ships)
		{
			if (!living(ship) || ship.empireId == self.empireId)
			{
				continue;
			}

			if (std::abs(ship.position - self.position) > weaponRange)
			{
				continue;
			}

			if (best == nullptr || ship.id < best->id)
			{
				best = &ship;
			}
		}

		return best;
	}

	const CombatShip* nearestLivingEnemy(const std::vector<CombatShip>& ships, const CombatShip& self)
	{
		const CombatShip* best = nullptr;
		float bestDist = std::numeric_limits<float>::infinity();
		for (const auto& ship : ships)
		{
			if (!living(ship) || ship.empireId == self.empireId)
			{
				continue;
			}

			const float dist = std::abs(ship.position - self.position);
			if (dist < bestDist || (dist == bestDist && best != nullptr && ship.id < best->id))
			{
				bestDist = dist;
				best = &ship;
			}
		}

		return best;
	}

	bool sideAlive(const std::vector<CombatShip>& ships, long long empireId)
	{
		for (const auto& ship : ships)
		{
			if (ship.empireId == empireId && living(ship))
			{
				return true;
			}
		}

		return false;
	}

	bool twoSidesAlive(const std::vector<CombatShip>& ships, long long empireA, long long empireB)
	{
		return sideAlive(ships, empireA) && sideAlive(ships, empireB);
	}

	std::vector<CombatShip> expandFleetHullsIntoCombatShips(
		const WorldData& world,
		const std::vector<long long>& fleetIds,
		long long sideAEmpire)
	{
		std::vector<CombatShip> ships;
		long long nextId = 1;
		for (const long long fleetId : fleetIds)
		{
			const Fleet* fleet = world.fleetById(fleetId);
			if (fleet == nullptr)
			{
				continue;
			}

			const float position = fleet->empireId == sideAEmpire 
				? 0.0f 
				: CombatEngagementLength;
			FleetApproach approach = FleetApproach::CloseToLaserRange;
			const EngagementPolicy* policy = activeEngagementPolicy(world, *fleet);
			if (policy != nullptr)
			{
				approach = policy->approach;
			}

			for (const auto& hull : fleet->hulls)
			{
				const ShipClass* shipClass = world.shipClassById(hull.classId);
				if (shipClass == nullptr)
				{
					continue;
				}

				const EmpireStats& stats = empireStats(world, fleet->empireId);
				ships.push_back(CombatShip{
					.id = nextId++,
					.fleetId = fleet->id,
					.empireId = fleet->empireId,
					.classId = hull.classId,
					.hullId = hull.id,
					.position = position,
					.range = shipClass->range * stats.laserRangeMultiplier,
					.missileRange = shipClass->missileRange,
					.accuracy = shipClass->accuracy,
					.laserDamage = static_cast<long long>(
						std::floor(static_cast<float>(shipClass->laserDamage) * stats.laserDamageMultiplier)),
					.missileDamage = shipClass->missileDamage,
					.missileTubes = shipClass->missileTubes,
					.missileReloadInnerTicks = shipClass->missileReloadInnerTicks,
					.magazine = hull.magazine,
					.missileSpeed = shipClass->missileSpeed,
					.shields = shipClass->maxShields,
					.hull = shipClass->maxHull,
					.closeRate = shipClass->closeRate,
					.approach = approach });
			}
		}

		return ships;
	}

	void closeShipsTowardNearestEnemy(std::vector<CombatShip>& ships)
	{
		std::vector<float> newPositions(ships.size());
		for (size_t i = 0; i < ships.size(); ++i)
		{
			const CombatShip& ship = ships[i];
			newPositions[i] = ship.position;
			if (!living(ship))
			{
				continue;
			}

			const CombatShip* enemy = nearestLivingEnemy(ships, ship);
			if (enemy == nullptr)
			{
				continue;
			}

			const float delta = enemy->position - ship.position;
			const float dist = std::abs(delta);
			const float stopRange =
				ship.missileRange > 0.0f ? ship.missileRange : ship.range;
			if (ship.approach == FleetApproach::HoldMissileRange && dist <= stopRange)
			{
				continue;
			}

			if (dist <= ship.closeRate)
			{
				newPositions[i] = enemy->position;
			}
			else
			{
				newPositions[i] = ship.position + (delta > 0.0f ? ship.closeRate : -ship.closeRate);
			}
		}

		for (size_t i = 0; i < ships.size(); ++i)
		{
			ships[i].position = newPositions[i];
		}
	}

	void fireLasersAtLowestEnemyInRange(std::vector<CombatShip>& ships, std::uint64_t& rng)
	{
		std::vector<long long> order;
		order.reserve(ships.size());
		for (const auto& ship : ships)
		{
			order.push_back(ship.id);
		}

		std::sort(order.begin(), order.end());
		for (const long long id : order)
		{
			CombatShip* shooter = nullptr;
			for (auto& ship : ships)
			{
				if (ship.id == id)
				{
					shooter = &ship;
					break;
				}
			}

			if (shooter == nullptr || !living(*shooter))
			{
				continue;
			}

			const CombatShip* targetRef = lowestLivingEnemyInWeaponRange(ships, *shooter, shooter->range);
			if (targetRef == nullptr)
			{
				continue;
			}

			CombatShip* target = nullptr;
			for (auto& ship : ships)
			{
				if (ship.id == targetRef->id)
				{
					target = &ship;
					break;
				}
			}

			if (target == nullptr)
			{
				continue;
			}

			if (hl::splitMix64Unit(rng) < shooter->accuracy)
			{
				applyLaserDamage(target->shields, target->hull, shooter->laserDamage);
			}
		}
	}

	CombatShip* combatShipById(std::vector<CombatShip>& ships, long long id)
	{
		for (auto& ship : ships)
		{
			if (ship.id == id)
			{
				return &ship;
			}
		}

		return nullptr;
	}

	void launchMissilesAtLowestEnemyInRange(
		std::vector<CombatShip>& ships,
		std::vector<CombatMissile>& missiles)
	{
		std::vector<long long> order;
		order.reserve(ships.size());
		for (const auto& ship : ships)
		{
			order.push_back(ship.id);
		}

		std::sort(order.begin(), order.end());
		for (const long long id : order)
		{
			CombatShip* shooter = combatShipById(ships, id);
			if (shooter == nullptr || !living(*shooter))
			{
				continue;
			}

			if (shooter->missileReloadRemaining > 0)
			{
				--shooter->missileReloadRemaining;
			}

			if (shooter->missileReloadRemaining > 0 || magazineUsed(shooter->magazine) < 1.f ||
				shooter->missileTubes <= 0 || shooter->missileSpeed <= 0.0f)
			{
				continue;
			}

			bool launched = false;
			for (long long round = 0; round < shooter->missileTubes; ++round)
			{
				const long long goodId = highestRangeLoadedCombatMissileGoodId(shooter->magazine);
				if (goodId == 0)
				{
					break;
				}

				const float fireRange =
					shooter->missileRange * combatMissileRangeScale(goodId);
				const CombatShip* targetRef =
					lowestLivingEnemyInWeaponRange(ships, *shooter, fireRange);
				if (targetRef == nullptr)
				{
					break;
				}

				if (!spendMagazineGood(shooter->magazine, goodId, 1.f))
				{
					break;
				}

				missiles.push_back(CombatMissile{
					.position = shooter->position,
					.speed = shooter->missileSpeed,
					.accuracy = shooter->accuracy,
					.damage = shooter->missileDamage,
					.targetId = targetRef->id });
				launched = true;
			}

			if (!launched)
			{
				continue;
			}

			shooter->missileReloadRemaining = shooter->missileReloadInnerTicks;
		}
	}

	void flyMissilesTowardTargets(
		std::vector<CombatShip>& ships,
		std::vector<CombatMissile>& missiles,
		std::uint64_t& rng)
	{
		std::vector<CombatMissile> stillFlying;
		stillFlying.reserve(missiles.size());
		for (auto& missile : missiles)
		{
			CombatShip* target = combatShipById(ships, missile.targetId);
			if (target == nullptr)
			{
				continue;
			}

			const float delta = target->position - missile.position;
			const float dist = std::abs(delta);
			if (dist > missile.speed)
			{
				missile.position += (delta > 0.0f ? missile.speed : -missile.speed);
				stillFlying.push_back(missile);
				continue;
			}

			if (!living(*target))
			{
				continue;
			}

			if (hl::splitMix64Unit(rng) < missile.accuracy)
			{
				applyLaserDamage(target->shields, target->hull, missile.damage);
			}
		}

		missiles = std::move(stillFlying);
	}

	void simulateCombatTicks(std::vector<CombatShip>& ships, std::uint64_t& rng, long long empireA, long long empireB)
	{
		std::vector<CombatMissile> missiles;
		for (int inner = 0; inner < CombatMaxInnerTicks; ++inner)
		{
			closeShipsTowardNearestEnemy(ships);
			if (twoSidesAlive(ships, empireA, empireB))
			{
				fireLasersAtLowestEnemyInRange(ships, rng);
			}

			flyMissilesTowardTargets(ships, missiles, rng);
			if (twoSidesAlive(ships, empireA, empireB))
			{
				launchMissilesAtLowestEnemyInRange(ships, missiles);
			}

			if (!twoSidesAlive(ships, empireA, empireB) && missiles.empty())
			{
				return;
			}
		}
	}

	void writeCombatHullsBackToFleet(Fleet& fleet, const std::vector<CombatShip>& ships)
	{
		std::vector<FleetHull> livingHulls;
		for (const auto& hull : fleet.hulls)
		{
			const CombatShip* combatShip = nullptr;
			for (const auto& ship : ships)
			{
				if (ship.fleetId == fleet.id && ship.hullId == hull.id)
				{
					combatShip = &ship;
					break;
				}
			}

			if (combatShip == nullptr || !living(*combatShip))
			{
				continue;
			}

			FleetHull updated = hull;
			updated.magazine = combatShip->magazine;
			livingHulls.push_back(updated);
		}

		fleet.hulls = std::move(livingHulls);
		syncFleetCompositionFromHulls(fleet);
	}

	void resolveSystemBattle(WorldData& world, long long systemId, std::vector<long long> fleetIds)
	{
		std::sort(fleetIds.begin(), fleetIds.end());
		std::set<long long> empires;
		for (const long long fleetId : fleetIds)
		{
			const Fleet* fleet = world.fleetById(fleetId);
			if (fleet != nullptr)
			{
				empires.insert(fleet->empireId);
			}
		}

		if (empires.size() < 2)
		{
			return;
		}

		if (!anyFleetOpensFire(world, fleetIds))
		{
			return;
		}

		for (auto a = empires.begin(); a != empires.end(); ++a)
		{
			auto b = a;
			++b;
			for (; b != empires.end(); ++b)
			{
				declareWarBetweenEmpires(world, *a, *b);
			}
		}

		const long long empireA = *empires.begin();
		const long long empireB = *std::next(empires.begin());
		const long long battleId = static_cast<long long>(world.battleReports.size()) + 1;
		std::vector<CombatShip> ships = expandFleetHullsIntoCombatShips(world, fleetIds, empireA);
		if (ships.empty())
		{
			return;
		}

		BattleReport report;
		report.id = battleId;
		report.tick = world.tick;
		report.systemId = systemId;
		for (const long long fleetId : fleetIds)
		{
			const Fleet* fleet = world.fleetById(fleetId);
			if (fleet == nullptr)
			{
				continue;
			}

			report.sides.push_back(BattleSideReport{
				.fleetId = fleet->id,
				.empireId = fleet->empireId,
				.before = fleet->composition,
				.missilesBefore = fleetMissileAmmo(*fleet) });
		}

		std::uint64_t rng = hl::splitMix64Seed({
			world.masterSeed,
			static_cast<std::uint64_t>(world.tick),
			static_cast<std::uint64_t>(battleId) });
		simulateCombatTicks(ships, rng, empireA, empireB);

		for (auto& side : report.sides)
		{
			Fleet* fleet = world.fleetById(side.fleetId);
			if (fleet == nullptr)
			{
				continue;
			}

			writeCombatHullsBackToFleet(*fleet, ships);
			side.after = fleet->composition;
			side.missilesAfter = fleetMissileAmmo(*fleet);
		}

		applyBattleSurvivorIntel(world, report);
		world.battleReports.push_back(std::move(report));
	}

}

	void applyLaserDamage(long long& shields, long long& hull, long long damage)
	{
		if (damage <= 0)
		{
			return;
		}

		if (damage <= shields)
		{
			shields -= damage;
			return;
		}

		damage -= shields;
		shields = 0;
		hull = std::max(0LL, hull - damage);
	}

	bool fleetAndTruckShareBattleMeeting(const WorldData& world, const Fleet& fleet, const Truck& truck)
	{
		if (!isTruckAtSystem(truck, fleet.systemId))
		{
			return false;
		}

		return moversAreWithinSystemBattleMeetingRadius(
			world,
			fleet.systemId,
			fleet.placeId,
			fleet.inSystem,
			truck.placeId,
			truck.inSystem);
	}

	int meetingClusterRoot(std::vector<int>& parent, int index)
	{
		while (parent[static_cast<size_t>(index)] != index)
		{
			parent[static_cast<size_t>(index)] = parent[static_cast<size_t>(parent[static_cast<size_t>(index)])];
			index = parent[static_cast<size_t>(index)];
		}

		return index;
	}

	void uniteMeetingClusters(std::vector<int>& parent, int a, int b)
	{
		const int rootA = meetingClusterRoot(parent, a);
		const int rootB = meetingClusterRoot(parent, b);
		if (rootA != rootB)
		{
			parent[static_cast<size_t>(rootB)] = rootA;
		}
	}

	std::vector<std::vector<long long>> clusterParkedFleetsIntoMeetings(
		const WorldData& world,
		const std::vector<long long>& fleetIds)
	{
		std::vector<std::vector<long long>> clusters;
		if (fleetIds.empty())
		{
			return clusters;
		}

		const Fleet* first = world.fleetById(fleetIds.front());
		if (first == nullptr)
		{
			return clusters;
		}

		if (world.systemLayoutBySystemId(first->systemId) == nullptr)
		{
			clusters.push_back(fleetIds);
			return clusters;
		}

		std::vector<int> parent(fleetIds.size());
		std::iota(parent.begin(), parent.end(), 0);
		for (size_t i = 0; i < fleetIds.size(); ++i)
		{
			const Fleet* fleetA = world.fleetById(fleetIds[i]);
			if (fleetA == nullptr)
			{
				continue;
			}

			for (size_t j = i + 1; j < fleetIds.size(); ++j)
			{
				const Fleet* fleetB = world.fleetById(fleetIds[j]);
				if (fleetB == nullptr)
				{
					continue;
				}

				if (moversAreWithinSystemBattleMeetingRadius(
						world,
						fleetA->systemId,
						fleetA->placeId,
						fleetA->inSystem,
						fleetB->placeId,
						fleetB->inSystem))
				{
					uniteMeetingClusters(parent, static_cast<int>(i), static_cast<int>(j));
				}
			}
		}

		std::map<int, std::vector<long long>> byRoot;
		for (size_t i = 0; i < fleetIds.size(); ++i)
		{
			byRoot[meetingClusterRoot(parent, static_cast<int>(i))].push_back(fleetIds[i]);
		}

		for (auto& [root, ids] : byRoot)
		{
			clusters.push_back(std::move(ids));
		}

		return clusters;
	}

	bool fleetHasWarshipHulls(const WorldData& world, const Fleet& fleet)
	{
		for (const auto& hull : fleet.hulls)
		{
			const ShipClass* shipClass = world.shipClassById(hull.classId);
			if (shipClass != nullptr && shipClassHasRole(*shipClass, HullRole::Warship))
			{
				return true;
			}
		}

		return false;
	}

	bool fleetEligibleForEncounterStance(const Fleet& fleet)
	{
		return fleet.motion.progress <= 0.0f && fleetShipCount(fleet) > 0;
	}

	long long crawlFromPlaceId(const Fleet& fleet)
	{
		if (fleet.placeId != 0)
		{
			return fleet.placeId;
		}

		if (inSystemMotionIsCrawling(fleet.inSystem))
		{
			return fleet.inSystem.fromPlaceId;
		}

		return 0;
	}

	long long hostilePlaceIdForFleet(const Fleet& fleet)
	{
		if (inSystemMotionIsCrawling(fleet.inSystem))
		{
			return fleet.inSystem.toPlaceId;
		}

		return fleet.placeId;
	}

	long long meetingHostileEmpireForFleet(const WorldData& world, const Fleet& fleet)
	{
		long long chosenFleetId = 0;
		long long hostileEmpire = 0;
		for (const auto& other : world.fleets)
		{
			if (other.id == fleet.id || other.systemId != fleet.systemId)
			{
				continue;
			}

			if (other.empireId == fleet.empireId || !fleetHasWarshipHulls(world, other))
			{
				continue;
			}

			if (fleetShipCount(other) == 0)
			{
				continue;
			}

			if (!empiresAreAtWar(world, fleet.empireId, other.empireId))
			{
				continue;
			}

			if (!moversAreWithinSystemBattleMeetingRadius(
					world,
					fleet.systemId,
					fleet.placeId,
					fleet.inSystem,
					other.placeId,
					other.inSystem))
			{
				continue;
			}

			if (chosenFleetId == 0 || other.id < chosenFleetId)
			{
				chosenFleetId = other.id;
				hostileEmpire = other.empireId;
			}
		}

		return hostileEmpire;
	}

	const Fleet* pickChaseHostileFleet(const WorldData& world, const Fleet& fleet)
	{
		const auto selfPosition = inSystemMoverPosition(
			world,
			fleet.systemId,
			fleet.placeId,
			fleet.inSystem);
		if (!selfPosition.has_value())
		{
			return nullptr;
		}

		const Fleet* best = nullptr;
		float bestDist = std::numeric_limits<float>::max();
		for (const auto& other : world.fleets)
		{
			if (other.id == fleet.id || other.systemId != fleet.systemId)
			{
				continue;
			}

			if (other.empireId == fleet.empireId || !fleetHasWarshipHulls(world, other))
			{
				continue;
			}

			if (fleetShipCount(other) == 0)
			{
				continue;
			}

			if (!empiresAreAtWar(world, fleet.empireId, other.empireId))
			{
				continue;
			}

			if (moversAreWithinSystemBattleMeetingRadius(
					world,
					fleet.systemId,
					fleet.placeId,
					fleet.inSystem,
					other.placeId,
					other.inSystem))
			{
				continue;
			}

			const auto otherPosition = inSystemMoverPosition(
				world,
				other.systemId,
				other.placeId,
				other.inSystem);
			if (!otherPosition.has_value())
			{
				continue;
			}

			const float dist = glm::length(*selfPosition - *otherPosition);
			if (best == nullptr || dist < bestDist || (dist == bestDist && other.id < best->id))
			{
				bestDist = dist;
				best = world.fleetById(other.id);
			}
		}

		return best;
	}

	long long pickNearestValidFleeJumpPlaceId(
		const WorldData& world,
		const Fleet& fleet,
		long long hostileEmpireId)
	{
		const SystemLayout* layout = world.systemLayoutBySystemId(fleet.systemId);
		if (layout == nullptr)
		{
			return 0;
		}

		const auto selfPosition = inSystemMoverPosition(
			world,
			fleet.systemId,
			fleet.placeId,
			fleet.inSystem);
		if (!selfPosition.has_value())
		{
			return 0;
		}

		long long bestPlaceId = 0;
		float bestDist = std::numeric_limits<float>::max();
		for (const auto& place : layout->places)
		{
			if (place.kind != SystemPlaceKind::Jump)
			{
				continue;
			}

			const StarSystem* neighbor = world.systemById(place.neighborSystemId);
			if (neighbor == nullptr || neighbor->empireId == hostileEmpireId)
			{
				continue;
			}

			const float dist = glm::length(place.position - *selfPosition);
			if (bestPlaceId == 0 || dist < bestDist || (dist == bestDist && place.id < bestPlaceId))
			{
				bestDist = dist;
				bestPlaceId = place.id;
			}
		}

		return bestPlaceId;
	}

	bool fleetIsFleeingMeetingCluster(const WorldData& world, const Fleet& fleet)
	{
		const EngagementPolicy* policy = activeEngagementPolicy(world, fleet);
		if (policy == nullptr || policy->encounterStance != EncounterStance::Flee)
		{
			return false;
		}

		if (meetingHostileEmpireForFleet(world, fleet) == 0)
		{
			return false;
		}

		if (!inSystemMotionIsCrawling(fleet.inSystem))
		{
			return false;
		}

		const SystemLayout* layout = world.systemLayoutBySystemId(fleet.systemId);
		if (layout == nullptr)
		{
			return false;
		}

		const SystemPlace* to = placeById(*layout, fleet.inSystem.toPlaceId);
		return to != nullptr && to->kind == SystemPlaceKind::Jump;
	}

	bool fleetExcludedFromEncounterMeeting(const WorldData& world, long long fleetId)
	{
		for (const long long id : world.encounterStanceFleeMeetingExclusions)
		{
			if (id == fleetId)
			{
				return true;
			}
		}

		const Fleet* fleet = world.fleetById(fleetId);
		return fleet != nullptr && fleetIsFleeingMeetingCluster(world, *fleet);
	}

	void destroyEnemyTrucksAtWeaponsFreeFleets(WorldData& world)
	{
		std::vector<long long> truckIdsToDestroy;
		for (const auto& fleet : world.fleets)
		{
			if (!isParked(fleet) || fleetShipCount(fleet) == 0)
			{
				continue;
			}

			const EngagementPolicy* policy = activeEngagementPolicy(world, fleet);
			if (policy == nullptr || !ruleOfEngagementOpensFire(policy->ruleOfEngagement))
			{
				continue;
			}

			for (const auto& truck : world.trucks)
			{
				if (truck.empireId == fleet.empireId || !fleetAndTruckShareBattleMeeting(world, fleet, truck))
				{
					continue;
				}

				declareWarBetweenEmpires(world, fleet.empireId, truck.empireId);
				truckIdsToDestroy.push_back(truck.id);
			}
		}

		std::sort(truckIdsToDestroy.begin(), truckIdsToDestroy.end());
		truckIdsToDestroy.erase(
			std::unique(truckIdsToDestroy.begin(), truckIdsToDestroy.end()),
			truckIdsToDestroy.end());
		for (const long long truckId : truckIdsToDestroy)
		{
			destroyTruckAndReduceInbound(world, truckId);
		}
	}

	void applyEncounterStance(WorldData& world)
	{
		world.encounterStanceFleeMeetingExclusions.clear();
		for (auto& fleet : world.fleets)
		{
			if (!fleetEligibleForEncounterStance(fleet) || !fleetHasWarshipHulls(world, fleet))
			{
				continue;
			}

			if (world.systemLayoutBySystemId(fleet.systemId) == nullptr)
			{
				continue;
			}

			const EngagementPolicy* policy = activeEngagementPolicy(world, fleet);
			if (policy == nullptr)
			{
				continue;
			}

			if (policy->encounterStance == EncounterStance::Flee)
			{
				const long long hostileEmpire = meetingHostileEmpireForFleet(world, fleet);
				if (hostileEmpire == 0)
				{
					continue;
				}

				const long long jumpPlaceId = pickNearestValidFleeJumpPlaceId(world, fleet, hostileEmpire);
				if (jumpPlaceId == 0)
				{
					continue;
				}

				const long long fromPlaceId = crawlFromPlaceId(fleet);
				if (fromPlaceId == 0)
				{
					continue;
				}

				if (fromPlaceId == jumpPlaceId && !inSystemMotionIsCrawling(fleet.inSystem))
				{
					world.encounterStanceFleeMeetingExclusions.push_back(fleet.id);
					continue;
				}

				if (startInSystemCrawl(
						world,
						fleet.systemId,
						fromPlaceId,
						jumpPlaceId,
						fleet.inSystem,
						fleet.placeId))
				{
					world.encounterStanceFleeMeetingExclusions.push_back(fleet.id);
				}

				continue;
			}

			if (policy->encounterStance != EncounterStance::Chase)
			{
				continue;
			}

			const Fleet* target = pickChaseHostileFleet(world, fleet);
			if (target == nullptr)
			{
				continue;
			}

			const long long targetPlaceId = hostilePlaceIdForFleet(*target);
			if (targetPlaceId == 0)
			{
				continue;
			}

			const long long fromPlaceId = crawlFromPlaceId(fleet);
			if (fromPlaceId == 0)
			{
				continue;
			}

			if (inSystemMotionIsCrawling(fleet.inSystem) && fleet.inSystem.fromPlaceId == fromPlaceId &&
				fleet.inSystem.toPlaceId == targetPlaceId)
			{
				continue;
			}

			startInSystemCrawl(
				world,
				fleet.systemId,
				fromPlaceId,
				targetPlaceId,
				fleet.inSystem,
				fleet.placeId);
		}

		std::sort(
			world.encounterStanceFleeMeetingExclusions.begin(),
			world.encounterStanceFleeMeetingExclusions.end());
		world.encounterStanceFleeMeetingExclusions.erase(
			std::unique(
				world.encounterStanceFleeMeetingExclusions.begin(),
				world.encounterStanceFleeMeetingExclusions.end()),
			world.encounterStanceFleeMeetingExclusions.end());
	}

	void resolveBattles(WorldData& world)
	{
		destroyEnemyTrucksAtWeaponsFreeFleets(world);

		std::map<long long, std::vector<long long>> parkedBySystem;
		for (const auto& fleet : world.fleets)
		{
			if (!isParked(fleet) || fleetShipCount(fleet) == 0)
			{
				continue;
			}

			if (fleetExcludedFromEncounterMeeting(world, fleet.id))
			{
				continue;
			}

			parkedBySystem[fleet.systemId].push_back(fleet.id);
		}

		for (auto& [systemId, fleetIds] : parkedBySystem)
		{
			for (auto& meeting : clusterParkedFleetsIntoMeetings(world, fleetIds))
			{
				resolveSystemBattle(world, systemId, meeting);
			}
		}
	}

}
