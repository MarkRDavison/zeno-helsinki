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
		float position{ 0.0f };
		float range{ 0.0f };
		float accuracy{ 0.0f };
		long long laserDamage{ 0 };
		long long shields{ 0 };
		long long hull{ 0 };
		float closeRate{ 0.0f };
	};

	bool isParked(const Fleet& fleet)
	{
		return !(fleet.motion.progress > 0.0f);
	}

	bool living(const CombatShip& ship)
	{
		return ship.hull > 0;
	}

	const CombatShip* lowestLivingEnemyInRange(const std::vector<CombatShip>& ships, const CombatShip& self)
	{
		const CombatShip* best = nullptr;
		for (const auto& ship : ships)
		{
			if (!living(ship) || ship.empireId == self.empireId)
			{
				continue;
			}

			if (std::abs(ship.position - self.position) > self.range)
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

	std::vector<CombatShip> expandClassCountsIntoCombatShips(
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

			for (const auto& row : fleet->composition)
			{
				const ShipClass* shipClass = world.shipClassById(row.classId);
				if (shipClass == nullptr)
				{
					continue;
				}

				for (long long i = 0; i < row.count; ++i)
				{
					ships.push_back(CombatShip{
						.id = nextId++,
						.fleetId = fleet->id,
						.empireId = fleet->empireId,
						.classId = row.classId,
						.position = position,
						.range = shipClass->range,
						.accuracy = shipClass->accuracy,
						.laserDamage = shipClass->laserDamage,
						.shields = shipClass->maxShields,
						.hull = shipClass->maxHull,
						.closeRate = shipClass->closeRate });
				}
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

			const CombatShip* targetRef = lowestLivingEnemyInRange(ships, *shooter);
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

	void simulateCombatTicks(std::vector<CombatShip>& ships, std::uint64_t& rng, long long empireA, long long empireB)
	{
		for (int inner = 0; inner < CombatMaxInnerTicks; ++inner)
		{
			if (!twoSidesAlive(ships, empireA, empireB))
			{
				return;
			}

			closeShipsTowardNearestEnemy(ships);
			fireLasersAtLowestEnemyInRange(ships, rng);
		}
	}

	std::vector<ClassCount> countSurvivingShipsByClass(const std::vector<CombatShip>& ships, long long fleetId)
	{
		std::map<long long, long long> counts;
		for (const auto& ship : ships)
		{
			if (ship.fleetId != fleetId || !living(ship))
			{
				continue;
			}

			counts[ship.classId] += 1;
		}

		std::vector<ClassCount> composition;
		for (const auto& [classId, count] : counts)
		{
			composition.push_back(ClassCount{ .classId = classId, .count = count });
		}

		return composition;
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
		std::vector<CombatShip> ships = expandClassCountsIntoCombatShips(world, fleetIds, empireA);
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
				.before = fleet->composition });
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

			fleet->composition = countSurvivingShipsByClass(ships, fleet->id);
			side.after = fleet->composition;
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
