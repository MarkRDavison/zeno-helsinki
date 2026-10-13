#include <World/Combat.hpp>
#include <World/TruckCargo.hpp>
#include <helsinki/System/Utils/SplitMix64.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <limits>
#include <map>
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

				ships.push_back(CombatShip{
					.id = nextId++,
					.fleetId = fleet->id,
					.empireId = fleet->empireId,
					.classId = hull.classId,
					.hullId = hull.id,
					.position = position,
					.range = shipClass->range,
					.missileRange = shipClass->missileRange,
					.accuracy = shipClass->accuracy,
					.laserDamage = shipClass->laserDamage,
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
				if (truck.empireId == fleet.empireId || !isTruckAtSystem(truck, fleet.systemId))
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

			parkedBySystem[fleet.systemId].push_back(fleet.id);
		}

		for (auto& [systemId, fleetIds] : parkedBySystem)
		{
			resolveSystemBattle(world, systemId, fleetIds);
		}
	}

}
