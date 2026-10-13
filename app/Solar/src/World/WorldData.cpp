#include <World/WorldData.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <algorithm>
#include <cmath>
#include <map>
#include <utility>

namespace slr
{

	bool shipClassHasRole(const ShipClass& shipClass, HullRole role)
	{
		return (shipClass.hullRoles & hullRoleBit(role)) != 0;
	}

	bool truckHasRole(const WorldData& world, const Truck& truck, HullRole role)
	{
		const ShipClass* shipClass = world.shipClassById(truck.classId);
		return shipClass != nullptr && shipClassHasRole(*shipClass, role);
	}

	bool truckHasCargoHaulRole(const WorldData& world, const Truck& truck)
	{
		return truckHasRole(world, truck, HullRole::CivilianCargo) ||
			truckHasRole(world, truck, HullRole::MilitaryCargo);
	}

	bool truckUsesMilitaryHaulPath(const WorldData& world, const Truck& truck)
	{
		return truckHasRole(world, truck, HullRole::MilitaryCargo) ||
			truckHasRole(world, truck, HullRole::Tender);
	}

	const Empire* WorldData::empireById(long long id) const
	{
		for (const auto& empire : empires)
		{
			if (empire.id == id)
			{
				return &empire;
			}
		}

		return nullptr;
	}

	Empire* WorldData::empireById(long long id)
	{
		return const_cast<Empire*>(static_cast<const WorldData*>(this)->empireById(id));
	}

	const StarSystem* WorldData::systemById(long long id) const
	{
		for (const auto& system : systems)
		{
			if (system.id == id)
			{
				return &system;
			}
		}

		return nullptr;
	}

	StarSystem* WorldData::systemById(long long id)
	{
		return const_cast<StarSystem*>(static_cast<const WorldData*>(this)->systemById(id));
	}

	const ShipClass* WorldData::shipClassById(long long id) const
	{
		for (const auto& shipClass : shipClasses)
		{
			if (shipClass.id == id)
			{
				return &shipClass;
			}
		}

		return nullptr;
	}

	ShipClass* WorldData::shipClassById(long long id)
	{
		return const_cast<ShipClass*>(static_cast<const WorldData*>(this)->shipClassById(id));
	}

	const Fleet* WorldData::fleetById(long long id) const
	{
		for (const auto& fleet : fleets)
		{
			if (fleet.id == id)
			{
				return &fleet;
			}
		}

		return nullptr;
	}

	Fleet* WorldData::fleetById(long long id)
	{
		return const_cast<Fleet*>(static_cast<const WorldData*>(this)->fleetById(id));
	}

	const HaulJob* WorldData::haulJobById(long long id) const
	{
		for (const auto& job : haulJobs)
		{
			if (job.id == id)
			{
				return &job;
			}
		}

		return nullptr;
	}

	HaulJob* WorldData::haulJobById(long long id)
	{
		return const_cast<HaulJob*>(static_cast<const WorldData*>(this)->haulJobById(id));
	}

	const Convoy* WorldData::convoyById(long long id) const
	{
		for (const auto& convoy : convoys)
		{
			if (convoy.id == id)
			{
				return &convoy;
			}
		}

		return nullptr;
	}

	Convoy* WorldData::convoyById(long long id)
	{
		return const_cast<Convoy*>(static_cast<const WorldData*>(this)->convoyById(id));
	}

	const Truck* WorldData::truckById(long long id) const
	{
		for (const auto& truck : trucks)
		{
			if (truck.id == id)
			{
				return &truck;
			}
		}

		return nullptr;
	}

	Truck* WorldData::truckById(long long id)
	{
		return const_cast<Truck*>(static_cast<const WorldData*>(this)->truckById(id));
	}

	const EngagementPolicy* WorldData::engagementPolicyById(long long id) const
	{
		for (const auto& policy : engagementPolicies)
		{
			if (policy.id == id)
			{
				return &policy;
			}
		}

		return nullptr;
	}

	EngagementPolicy* WorldData::engagementPolicyById(long long id)
	{
		return const_cast<EngagementPolicy*>(static_cast<const WorldData*>(this)->engagementPolicyById(id));
	}

	const ResearchProgress* WorldData::researchProgressByEmpireAndProject(
		long long empireId,
		long long projectId) const
	{
		for (const auto& row : researchProgress)
		{
			if (row.empireId == empireId && row.projectId == projectId)
			{
				return &row;
			}
		}

		return nullptr;
	}

	ResearchProgress* WorldData::researchProgressByEmpireAndProject(long long empireId, long long projectId)
	{
		return const_cast<ResearchProgress*>(
			static_cast<const WorldData*>(this)->researchProgressByEmpireAndProject(empireId, projectId));
	}

	bool ruleOfEngagementOpensFire(RuleOfEngagement rule)
	{
		return rule == RuleOfEngagement::FireIfEnemySeen;
	}

	const char* ruleOfEngagementName(RuleOfEngagement rule)
	{
		switch (rule)
		{
		case RuleOfEngagement::FireIfEnemySeen:
			return "fire if enemy seen";
		case RuleOfEngagement::DoNotFireUnlessFiredUpon:
			return "do not fire unless fired upon";
		}

		return "unknown";
	}

	const char* fleetApproachName(FleetApproach approach)
	{
		switch (approach)
		{
		case FleetApproach::HoldMissileRange:
			return "hold missile range";
		case FleetApproach::CloseToLaserRange:
			return "close to lasers";
		}

		return "unknown";
	}

	const EngagementPolicy* activeEngagementPolicy(const WorldData& world, const Fleet& fleet)
	{
		if (fleet.policyId != 0)
		{
			const EngagementPolicy* overridePolicy = world.engagementPolicyById(fleet.policyId);
			if (overridePolicy != nullptr && overridePolicy->empireId == fleet.empireId)
			{
				return overridePolicy;
			}
		}

		const Empire* empire = world.empireById(fleet.empireId);
		if (empire == nullptr || empire->defaultPolicyId == 0)
		{
			return nullptr;
		}

		const EngagementPolicy* defaultPolicy = world.engagementPolicyById(empire->defaultPolicyId);
		if (defaultPolicy == nullptr || defaultPolicy->empireId != fleet.empireId)
		{
			return nullptr;
		}

		return defaultPolicy;
	}

	bool anyFleetOpensFire(const WorldData& world, const std::vector<long long>& fleetIds)
	{
		for (const long long fleetId : fleetIds)
		{
			const Fleet* fleet = world.fleetById(fleetId);
			if (fleet == nullptr)
			{
				continue;
			}

			const EngagementPolicy* policy = activeEngagementPolicy(world, *fleet);
			if (policy != nullptr && ruleOfEngagementOpensFire(policy->ruleOfEngagement) &&
				fleetHasJoinedMissileAmmo(*fleet))
			{
				return true;
			}
		}

		return false;
	}

	GoodAmount getStockpileAmount(const StarSystem& system, long long goodId)
	{
		for (const auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				return stock.goodAmount;
			}
		}

		return 0.f;
	}

	GoodAmount locatedGoodAmount(const WorldData& world, long long goodId)
	{
		GoodAmount total = 0.f;
		for (const auto& system : world.systems)
		{
			total += getStockpileAmount(system, goodId);
		}

		for (const auto& truck : world.trucks)
		{
			if (truck.cargoGoodId == goodId)
			{
				total += truck.cargoAmount;
			}
		}

		return total;
	}

	GoodAmount getInbound(const StarSystem& system, long long goodId)
	{
		for (const auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				return stock.inbound;
			}
		}

		return 0;
	}

	GoodAmount getNeed(const StarSystem& system, long long goodId)
	{
		for (const auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				return stock.need;
			}
		}

		return 0;
	}

	GoodAmount uncoveredNeed(const StarSystem& system, long long goodId)
	{
		const GoodAmount inbound = getInbound(system, goodId);
		const GoodAmount need = getNeed(system, goodId);
		if (need <= inbound)
		{
			return 0.f;
		}

		return need - inbound;
	}

	GoodAmount quantityForGood(const std::vector<GoodQuantity>& rows, long long goodId)
	{
		GoodAmount total = 0.f;
		for (const auto& row : rows)
		{
			if (row.goodId == goodId)
			{
				total += row.amount;
			}
		}

		return total;
	}

	GoodAmount mineOutputPerTick(const Mine& mine, long long goodId)
	{
		GoodAmount total = 0.f;
		for (const auto& output : mine.outputsPerTick)
		{
			if (output.goodId == goodId)
			{
				total += output.amountPerTick;
			}
		}

		return total;
	}

	GoodAmount mineMaxStockpile(const Mine& mine, long long goodId)
	{
		GoodAmount total = 0.f;
		for (const auto& output : mine.outputsPerTick)
		{
			if (output.goodId == goodId)
			{
				total += output.maxStockpile;
			}
		}

		return total;
	}

	const FactoryRecipe* factoryRecipeById(long long recipeId)
	{
		static const FactoryRecipe missileEngineMk1{
			.recipeId = FactoryRecipeId_MissileEngineMk1,
			.inputsPerTick = { GoodQuantity{ .goodId = SteelGoodId, .amount = FactoryAvonEngineSteelPerTick } },
			.outputGoodId = MissileEngineMk1GoodId,
			.outputAmountPerTick = FactoryAvonComponentOutputPerTick,
			.outputMaxStockpile = AvonComponentMaxStockpile };
		static const FactoryRecipe missileWarheadMk1{
			.recipeId = FactoryRecipeId_MissileWarheadMk1,
			.inputsPerTick = { GoodQuantity{ .goodId = SteelGoodId, .amount = FactoryAvonWarheadSteelPerTick } },
			.outputGoodId = MissileWarheadMk1GoodId,
			.outputAmountPerTick = FactoryAvonComponentOutputPerTick,
			.outputMaxStockpile = AvonComponentMaxStockpile };
		static const FactoryRecipe missileSeekerMk1{
			.recipeId = FactoryRecipeId_MissileSeekerMk1,
			.inputsPerTick = { GoodQuantity{ .goodId = SteelGoodId, .amount = FactoryAvonSeekerSteelPerTick } },
			.outputGoodId = MissileSeekerMk1GoodId,
			.outputAmountPerTick = FactoryAvonComponentOutputPerTick,
			.outputMaxStockpile = AvonComponentMaxStockpile };
		static const FactoryRecipe prototypeMissileMk1{
			.recipeId = FactoryRecipeId_PrototypeMissileMk1,
			.inputsPerTick = {
				GoodQuantity{
					.goodId = MissileEngineMk1GoodId,
					.amount = FactoryAvonAssemblerComponentInputPerTick },
				GoodQuantity{
					.goodId = MissileWarheadMk1GoodId,
					.amount = FactoryAvonAssemblerComponentInputPerTick },
				GoodQuantity{
					.goodId = MissileSeekerMk1GoodId,
					.amount = FactoryAvonAssemblerComponentInputPerTick } },
			.outputGoodId = PrototypeMissileMk1GoodId,
			.outputAmountPerTick = FactoryAvonAssemblerOutputPerTick,
			.outputMaxStockpile = AvonPrototypeMissileMaxStockpile,
			.ticksPerOutput = FactoryAvonAssemblerTicksPerOutput };
		static const FactoryRecipe missileEngineMk2{
			.recipeId = FactoryRecipeId_MissileEngineMk2,
			.inputsPerTick = { GoodQuantity{ .goodId = SteelGoodId, .amount = FactoryAvonEngineSteelPerTick } },
			.outputGoodId = MissileEngineMk2GoodId,
			.outputAmountPerTick = FactoryAvonComponentOutputPerTick,
			.outputMaxStockpile = AvonComponentMaxStockpile };
		static const FactoryRecipe prototypeMissileMk2{
			.recipeId = FactoryRecipeId_PrototypeMissileMk2,
			.inputsPerTick = {
				GoodQuantity{
					.goodId = MissileEngineMk2GoodId,
					.amount = FactoryAvonAssemblerComponentInputPerTick },
				GoodQuantity{
					.goodId = MissileWarheadMk1GoodId,
					.amount = FactoryAvonAssemblerComponentInputPerTick },
				GoodQuantity{
					.goodId = MissileSeekerMk1GoodId,
					.amount = FactoryAvonAssemblerComponentInputPerTick } },
			.outputGoodId = PrototypeMissileMk2GoodId,
			.outputAmountPerTick = FactoryAvonAssemblerOutputPerTick,
			.outputMaxStockpile = AvonPrototypeMissileMaxStockpile,
			.ticksPerOutput = FactoryAvonAssemblerTicksPerOutput };

		switch (recipeId)
		{
		case FactoryRecipeId_MissileEngineMk1:
			return &missileEngineMk1;
		case FactoryRecipeId_MissileWarheadMk1:
			return &missileWarheadMk1;
		case FactoryRecipeId_MissileSeekerMk1:
			return &missileSeekerMk1;
		case FactoryRecipeId_PrototypeMissileMk1:
			return &prototypeMissileMk1;
		case FactoryRecipeId_MissileEngineMk2:
			return &missileEngineMk2;
		case FactoryRecipeId_PrototypeMissileMk2:
			return &prototypeMissileMk2;
		default:
			return nullptr;
		}
	}

	void applyFactoryRecipe(Factory& factory, const FactoryRecipe& recipe)
	{
		factory.inputsPerTick = recipe.inputsPerTick;
		factory.outputGoodId = recipe.outputGoodId;
		factory.outputAmountPerTick = recipe.outputAmountPerTick;
		factory.outputMaxStockpile = recipe.outputMaxStockpile;
		factory.ticksPerOutput = recipe.ticksPerOutput;
		factory.remainingOutputTicks = 0;
		factory.recipeId = recipe.recipeId;
	}

	Factory makeFactoryFromRecipe(
		long long systemId,
		long long empireId,
		const FactoryRecipe& recipe,
		std::vector<GoodQuantity> desiredPiles)
	{
		Factory factory{
			.systemId = systemId,
			.empireId = empireId,
			.desiredPiles = std::move(desiredPiles) };
		applyFactoryRecipe(factory, recipe);
		if (const ProductRampSpec* rampSpec = productRampSpecForGood(recipe.outputGoodId))
		{
			factory.productEfficiencyPerThousand = rampSpec->startEfficiencyPerThousand;
		}

		return factory;
	}

	void retargetFactoryRecipe(Factory& factory, const FactoryRecipe& recipe)
	{
		if (factory.outputGoodId == recipe.outputGoodId)
		{
			return;
		}

		const ProductRampSpec* rampSpec = productRampSpecForGood(recipe.outputGoodId);
		const long long startEfficiency = rampSpec != nullptr
			? rampSpec->startEfficiencyPerThousand
			: FactoryStartEfficiencyPerThousand;
		if (goodsAreDeclaredVariants(factory.outputGoodId, recipe.outputGoodId))
		{
			const long long kept = factory.productEfficiencyPerThousand * FactoryVariantKeepPerThousand /
				FactoryFullEfficiencyPerThousand;
			factory.productEfficiencyPerThousand = kept > startEfficiency ? kept : startEfficiency;
		}
		else
		{
			factory.productEfficiencyPerThousand = startEfficiency;
		}

		factory.productRampUnitsProduced = 0.f;
		applyFactoryRecipe(factory, recipe);
	}

	const ProductRampSpec* productRampSpecForGood(long long goodId)
	{
		static const ProductRampSpec missileRamp{
			.goodId = MissileGoodId,
			.startEfficiencyPerThousand = FactoryStartEfficiencyPerThousand,
			.maxEfficiencyPerThousand = FactoryFullEfficiencyPerThousand };
		static const ProductRampSpec prototypeMissileRamp{
			.goodId = PrototypeMissileMk1GoodId,
			.startEfficiencyPerThousand = FactoryStartEfficiencyPerThousand,
			.maxEfficiencyPerThousand = FactoryFullEfficiencyPerThousand };

		if (goodId == missileRamp.goodId)
		{
			return &missileRamp;
		}

		if (goodId == PrototypeMissileMk1GoodId || goodId == PrototypeMissileMk2GoodId)
		{
			return &prototypeMissileRamp;
		}

		return nullptr;
	}

	GoodAmount factoryEffectiveAmount(GoodAmount ratedAmount, long long efficiencyPerThousand)
	{
		if (ratedAmount <= 0.f || efficiencyPerThousand <= 0)
		{
			return 0.f;
		}

		return ratedAmount * static_cast<float>(efficiencyPerThousand) /
			static_cast<float>(FactoryFullEfficiencyPerThousand);
	}

	void advanceProductRampAfterProduction(
		Factory& factory,
		const ProductRampSpec& spec,
		GoodAmount outputAdded)
	{
		if (outputAdded <= 0.f)
		{
			return;
		}

		factory.productRampUnitsProduced += outputAdded;
		while (factory.productRampUnitsProduced >= FactoryRampUnitsPerEfficiencyStep &&
			factory.productEfficiencyPerThousand < spec.maxEfficiencyPerThousand)
		{
			factory.productRampUnitsProduced -= FactoryRampUnitsPerEfficiencyStep;
			const long long next = factory.productEfficiencyPerThousand + FactoryRampEfficiencyStepPerThousand;
			factory.productEfficiencyPerThousand =
				next < spec.maxEfficiencyPerThousand ? next : spec.maxEfficiencyPerThousand;
		}
	}

	GoodAmount consumerDesiredPile(const WorldData& world, long long systemId, long long goodId)
	{
		GoodAmount desired = 0.f;
		for (const auto& yard : world.yards)
		{
			if (yard.systemId == systemId)
			{
				desired += quantityForGood(yard.desiredPiles, goodId);
				desired += quantityForGood(yard.populationReservedPiles, goodId);
			}
		}

		for (const auto& factory : world.factories)
		{
			if (factory.systemId == systemId)
			{
				desired += quantityForGood(factory.desiredPiles, goodId);
			}
		}

		return desired;
	}

	GoodAmount stockpileAvailableForYardConsume(const WorldData& world, long long systemId, long long goodId)
	{
		GoodAmount reserved = 0.f;
		for (const auto& yard : world.yards)
		{
			if (yard.systemId == systemId)
			{
				reserved += quantityForGood(yard.populationReservedPiles, goodId);
			}
		}

		const StarSystem* system = world.systemById(systemId);
		if (system == nullptr)
		{
			return 0;
		}

		return std::max(0.f, getStockpileAmount(*system, goodId) - reserved);
	}

	void postNeedFromConsumerShortfall(WorldData& world, StarSystem& system, long long goodId)
	{
		const GoodAmount desired = consumerDesiredPile(world, system.id, goodId);
		const GoodAmount stockpile = getStockpileAmount(system, goodId);
		GoodAmount shortfall = desired - stockpile;
		if (shortfall < 0.f)
		{
			shortfall = 0.f;
		}

		const GoodAmount posted = (shortfall >= truckloadForGood(goodId)) ? shortfall : 0.f;
		for (auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				stock.need = posted;
				return;
			}
		}

		if (posted == 0.f)
		{
			return;
		}

		system.stocks.push_back(GoodStock{ .goodId = goodId, .need = posted });
	}

	void postNeedsFromConsumerShortfalls(WorldData& world, StarSystem& system)
	{
		for (const auto& yard : world.yards)
		{
			if (yard.systemId != system.id)
			{
				continue;
			}

			for (const auto& pile : yard.desiredPiles)
			{
				postNeedFromConsumerShortfall(world, system, pile.goodId);
			}

			for (const auto& pile : yard.populationReservedPiles)
			{
				postNeedFromConsumerShortfall(world, system, pile.goodId);
			}

			for (const auto& pile : yard.populationConsumePerTick)
			{
				postNeedFromConsumerShortfall(world, system, pile.goodId);
			}
		}

		for (const auto& factory : world.factories)
		{
			if (factory.systemId != system.id)
			{
				continue;
			}

			for (const auto& pile : factory.desiredPiles)
			{
				postNeedFromConsumerShortfall(world, system, pile.goodId);
			}
		}
	}

	bool useFromStockpile(WorldData& world, long long systemId, long long goodId, GoodAmount amount)
	{
		if (amount <= 0.f)
		{
			return false;
		}

		StarSystem* system = world.systemById(systemId);
		if (system == nullptr)
		{
			return false;
		}

		for (auto& stock : system->stocks)
		{
			if (stock.goodId == goodId && stock.goodAmount >= amount)
			{
				stock.goodAmount -= amount;
				stock.deferredNeed += amount;
				stock.deferredNeedHeldOneTick = false;
				return true;
			}
		}

		return false;
	}

	void applyDeferredNeedPosts(WorldData& world)
	{
		for (auto& system : world.systems)
		{
			for (auto& stock : system.stocks)
			{
				const long long truckload = truckloadForGood(stock.goodId);
				if (stock.deferredNeedHeldOneTick && stock.deferredNeed >= truckload)
				{
					stock.need += stock.deferredNeed;
					stock.deferredNeed = 0;
					stock.deferredNeedHeldOneTick = false;
				}
				else if (stock.deferredNeed > 0)
				{
					stock.deferredNeedHeldOneTick = true;
				}
			}
		}
	}

	bool isTruckAtSystem(const Truck& truck, long long systemId)
	{
		if (truck.motion.progress != 0.0f || truck.motion.hop < 0 ||
			truck.motion.hop >= static_cast<int>(truck.motion.route.size()))
		{
			return false;
		}

		return truck.motion.route[static_cast<size_t>(truck.motion.hop)] == systemId;
	}

	long long countTrucksAtSystem(const WorldData& world, long long systemId)
	{
		long long count = 0;
		for (const auto& truck : world.trucks)
		{
			if (isTruckAtSystem(truck, systemId))
			{
				++count;
			}
		}

		return count;
	}

	long long fleetShipCount(const Fleet& fleet)
	{
		long long count = 0;
		for (const auto& row : fleet.composition)
		{
			count += row.count;
		}

		return count;
	}

	bool fleetIsParked(const Fleet& fleet)
	{
		return !(fleet.motion.progress > 0.0f);
	}

	long long classCountFor(const std::vector<ClassCount>& composition, long long classId)
	{
		for (const auto& row : composition)
		{
			if (row.classId == classId)
			{
				return row.count;
			}
		}

		return 0;
	}

	long long nextFleetHullId(const WorldData& world)
	{
		long long nextId = 1;
		for (const auto& fleet : world.fleets)
		{
			for (const auto& hull : fleet.hulls)
			{
				if (hull.id >= nextId)
				{
					nextId = hull.id + 1;
				}
			}
		}

		for (const auto& yard : world.yards)
		{
			for (const auto& job : yard.hullRefitQueue)
			{
				if (job.hullId >= nextId)
				{
					nextId = job.hullId + 1;
				}
			}

			for (const auto& slip : yard.slips)
			{
				if (slip.refitHullId >= nextId)
				{
					nextId = slip.refitHullId + 1;
				}
			}
		}

		return nextId;
	}

	long long nextFleetId(const WorldData& world)
	{
		long long nextId = 1;
		for (const auto& fleet : world.fleets)
		{
			if (fleet.id >= nextId)
			{
				nextId = fleet.id + 1;
			}
		}

		return nextId;
	}

	long long fleetMissileAmmo(const Fleet& fleet)
	{
		long long ammo = 0;
		for (const auto& hull : fleet.hulls)
		{
			ammo += static_cast<long long>(magazineUsed(hull.magazine));
		}

		return ammo;
	}

	bool fleetHasJoinedMissileAmmo(const Fleet& fleet)
	{
		return fleetMissileAmmo(fleet) > 0;
	}

	long long fleetMissileAmmoForClass(const Fleet& fleet, long long classId)
	{
		long long ammo = 0;
		for (const auto& hull : fleet.hulls)
		{
			if (hull.classId == classId)
			{
				ammo += static_cast<long long>(magazineUsed(hull.magazine));
			}
		}

		return ammo;
	}

	long long fleetMagazineMaxForClass(const WorldData& world, const Fleet& fleet, long long classId)
	{
		const ShipClass* shipClass = world.shipClassById(classId);
		if (shipClass == nullptr)
		{
			return 0;
		}

		long long hulls = 0;
		for (const auto& hull : fleet.hulls)
		{
			if (hull.classId == classId)
			{
				++hulls;
			}
		}

		return hulls * shipClass->magazineSize;
	}

	long long fleetWarshipMagazineCapacity(const WorldData& world, const Fleet& fleet)
	{
		long long capacity = 0;
		for (const auto& hull : fleet.hulls)
		{
			const ShipClass* shipClass = world.shipClassById(hull.classId);
			if (shipClass == nullptr || !shipClassHasRole(*shipClass, HullRole::Warship))
			{
				continue;
			}

			if (shipClass->magazineSize > capacity)
			{
				capacity = shipClass->magazineSize;
			}
		}

		return capacity > 0 ? capacity : 80;
	}

	DesiredCargo desiredCargoForFleetClass(const Fleet& fleet, long long classId)
	{
		for (const auto& row : fleet.magazineCargoByClass)
		{
			if (row.classId == classId)
			{
				return row.desired;
			}
		}

		return desiredCargoFillRemaining(MissileGoodId);
	}

	void ensureFleetClassDesiredCargo(Fleet& fleet, long long classId)
	{
		for (const auto& row : fleet.magazineCargoByClass)
		{
			if (row.classId == classId)
			{
				return;
			}
		}

		fleet.magazineCargoByClass.push_back(ClassDesiredCargo{
			.classId = classId,
			.desired = desiredCargoFillRemaining(MissileGoodId) });
	}

	void setFleetClassDesiredCargo(Fleet& fleet, long long classId, DesiredCargo desired)
	{
		for (auto& row : fleet.magazineCargoByClass)
		{
			if (row.classId == classId)
			{
				row.desired = std::move(desired);
				return;
			}
		}

		fleet.magazineCargoByClass.push_back(ClassDesiredCargo{
			.classId = classId,
			.desired = std::move(desired) });
	}

	std::vector<GoodQuantity> truckCargoAsQuantities(const Truck& truck)
	{
		if (truck.cargoGoodId == 0 || truck.cargoAmount <= 0.f)
		{
			return {};
		}

		return { GoodQuantity{ .goodId = truck.cargoGoodId, .amount = truck.cargoAmount } };
	}

	bool hullWantsCombatMissileGood(
		const DesiredCargo& magazineCargo,
		const FleetHull& hull,
		GoodAmount magazineSize,
		long long goodId)
	{
		if (!isCombatMissileGood(goodId) || magazineSize < 1.f)
		{
			return false;
		}

		const std::vector<GoodQuantity> available{
			GoodQuantity{ .goodId = goodId, .amount = 1.f } };
		return nextDesiredCargoGoodId(magazineCargo, hull.magazine, magazineSize, available) ==
			goodId;
	}

	void syncFleetCompositionFromHulls(Fleet& fleet)
	{
		std::map<long long, long long> counts;
		for (const auto& hull : fleet.hulls)
		{
			counts[hull.classId] += 1;
		}

		fleet.composition.clear();
		for (const auto& [classId, count] : counts)
		{
			fleet.composition.push_back(ClassCount{ .classId = classId, .count = count });
		}
	}

	bool removeFleetHull(Fleet& fleet, long long hullId)
	{
		for (auto hull = fleet.hulls.begin(); hull != fleet.hulls.end(); ++hull)
		{
			if (hull->id != hullId)
			{
				continue;
			}

			fleet.hulls.erase(hull);
			syncFleetCompositionFromHulls(fleet);
			return true;
		}

		return false;
	}

	void appendMagazineHulls(
		WorldData& world,
		Fleet& fleet,
		long long classId,
		long long count,
		long long missileAmmo)
	{
		const ShipClass* shipClass = world.shipClassById(classId);
		if (shipClass == nullptr || count <= 0)
		{
			return;
		}

		ensureFleetClassDesiredCargo(fleet, shipClass->id);

		for (long long i = 0; i < count; ++i)
		{
			FleetHull hull{
				.id = nextFleetHullId(world),
				.classId = shipClass->id };
			if (missileAmmo > 0)
			{
				addMagazineGood(hull.magazine, MissileGoodId, static_cast<GoodAmount>(missileAmmo));
			}

			fleet.hulls.push_back(std::move(hull));
		}

		syncFleetCompositionFromHulls(fleet);
	}

	void appendFullMagazineHulls(WorldData& world, Fleet& fleet, long long classId, long long count)
	{
		const ShipClass* shipClass = world.shipClassById(classId);
		if (shipClass == nullptr)
		{
			return;
		}

		appendMagazineHulls(world, fleet, classId, count, shipClass->magazineSize);
	}

	void appendEmptyMagazineHulls(WorldData& world, Fleet& fleet, long long classId, long long count)
	{
		appendMagazineHulls(world, fleet, classId, count, 0);
	}

	bool hasHyperlane(const WorldData& world, long long a, long long b)
	{
		if (a > b)
		{
			std::swap(a, b);
		}

		for (const auto& lane : world.hyperlanes)
		{
			if (lane.a == a && lane.b == b)
			{
				return true;
			}
		}

		return false;
	}

	const HaulJob* haulJobFor(const WorldData& world, long long destinationSystemId, long long goodId)
	{
		for (const auto& job : world.haulJobs)
		{
			if (job.destinationSystemId == destinationSystemId && job.goodId == goodId)
			{
				return &job;
			}
		}

		return nullptr;
	}

	long long truckSystemId(const Truck& truck)
	{
		if (truck.motion.progress == 0.0f && truck.motion.hop >= 0 &&
			truck.motion.hop < static_cast<int>(truck.motion.route.size()))
		{
			return truck.motion.route[static_cast<size_t>(truck.motion.hop)];
		}

		long long fromId = 0;
		long long toId = 0;
		if (hopEndpoints(truck.motion, fromId, toId))
		{
			return fromId;
		}

		return 0;
	}

	namespace
	{

		bool systemCanSupplyGood(const WorldData& world, const StarSystem& system, long long goodId)
		{
			if (getStockpileAmount(system, goodId) >= truckloadForGood(goodId))
			{
				return true;
			}

			for (const auto& mine : world.mines)
			{
				if (mine.systemId == system.id && mineOutputPerTick(mine, goodId) > 0)
				{
					return true;
				}
			}

			return false;
		}

		long long empireIdForHaulDestination(const WorldData& world, long long systemId)
		{
			for (const auto& yard : world.yards)
			{
				if (yard.systemId == systemId)
				{
					return yard.empireId;
				}
			}

			for (const auto& factory : world.factories)
			{
				if (factory.systemId == systemId)
				{
					return factory.empireId;
				}
			}

			return 0;
		}

		GoodAmount emptyAssignedOnJob(const WorldData& world, long long jobId, long long goodId)
		{
			const GoodAmount truckload = truckloadForGood(goodId);
			GoodAmount reserved = 0.f;
			for (const auto& truck : world.trucks)
			{
				if (truck.haulJobId == jobId && truck.cargoAmount <= 0.f)
				{
					reserved += truckload;
				}
			}

			return reserved;
		}

		GoodAmount remainingAcceptRoom(const WorldData& world, const StarSystem& destination, const HaulJob& job)
		{
			const GoodAmount uncovered = uncoveredNeed(destination, job.goodId);
			const GoodAmount reserved = emptyAssignedOnJob(world, job.id, job.goodId);
			if (uncovered <= reserved)
			{
				return 0.f;
			}

			return uncovered - reserved;
		}

		bool anyTruckAssignedToJob(const WorldData& world, long long jobId)
		{
			for (const auto& truck : world.trucks)
			{
				if (truck.haulJobId == jobId)
				{
					return true;
				}
			}

			return false;
		}

	}

	long long nearestSupplySystemId(
		const WorldData& world,
		long long fromSystemId,
		long long goodId,
		long long destinationSystemId,
		long long viewerEmpireId,
		bool seesEscort,
		bool isMilitary)
	{
		auto pick = [&](bool allowDestination) -> long long
		{
			long long bestId = 0;
			long long bestCost = 0;
			bool found = false;
			for (const auto& system : world.systems)
			{
				if (!allowDestination && system.id == destinationSystemId)
				{
					continue;
				}

				if (!systemCanSupplyGood(world, system, goodId))
				{
					continue;
				}

				const long long toSupply = haulLanePathCost(
					world,
					fromSystemId,
					system.id,
					viewerEmpireId,
					seesEscort,
					isMilitary);
				if (toSupply < 0)
				{
					continue;
				}

				const long long toDest = haulLanePathCost(
					world,
					system.id,
					destinationSystemId,
					viewerEmpireId,
					seesEscort,
					isMilitary);
				if (toDest < 0)
				{
					continue;
				}

				const long long cost = toSupply + toDest;
				if (!found || cost < bestCost || (cost == bestCost && system.id < bestId))
				{
					found = true;
					bestCost = cost;
					bestId = system.id;
				}
			}

			return bestId;
		};

		const long long skipDest = pick(false);
		if (skipDest != 0)
		{
			return skipDest;
		}

		return pick(true);
	}

	void parkTruckAt(Truck& truck, long long systemId)
	{
		truck.motion.route = { systemId };
		truck.motion.hop = 0;
		truck.motion.progress = 0.0f;
		truck.motion.direction = 1;
	}

	void syncHaulJobsFromUncoveredNeed(WorldData& world)
	{
		long long nextId = 1;
		for (const auto& job : world.haulJobs)
		{
			if (job.id >= nextId)
			{
				nextId = job.id + 1;
			}
		}

		for (const auto& system : world.systems)
		{
			for (const auto& stock : system.stocks)
			{
				HaulJob* existing = nullptr;
				for (auto& job : world.haulJobs)
				{
					if (job.destinationSystemId == system.id && job.goodId == stock.goodId)
					{
						existing = &job;
						break;
					}
				}

				const GoodAmount truckload = truckloadForGood(stock.goodId);
				GoodAmount remaining = 0.f;
				if (existing != nullptr)
				{
					remaining = remainingAcceptRoom(world, system, *existing);
				}
				else
				{
					remaining = uncoveredNeed(system, stock.goodId);
				}

				const bool assigned = existing != nullptr && anyTruckAssignedToJob(world, existing->id);
				if (remaining < truckload && !assigned)
				{
					continue;
				}

				const long long empireId = empireIdForHaulDestination(world, system.id);
				if (existing != nullptr)
				{
					existing->amount = remaining;
					if (existing->empireId == 0)
					{
						existing->empireId = empireId;
					}
				}
				else if (remaining >= truckload)
				{
					world.haulJobs.push_back(HaulJob{
						.id = nextId,
						.destinationSystemId = system.id,
						.empireId = empireId,
						.goodId = stock.goodId,
						.amount = remaining });
					++nextId;
				}
			}
		}

		world.haulJobs.erase(
			std::remove_if(
				world.haulJobs.begin(),
				world.haulJobs.end(),
				[&world](const HaulJob& job)
				{
					const StarSystem* system = world.systemById(job.destinationSystemId);
					if (system == nullptr)
					{
						return true;
					}

					if (anyTruckAssignedToJob(world, job.id))
					{
						return false;
					}

					return remainingAcceptRoom(world, *system, job) < truckloadForGood(job.goodId);
				}),
			world.haulJobs.end());
	}

	long long truckCountToCarryAmountInOneGo(GoodAmount amount, long long goodId)
	{
		if (amount <= 0.f)
		{
			return 0;
		}

		const GoodAmount truckload = truckloadForGood(goodId);
		return static_cast<long long>(std::ceil(amount / truckload));
	}

	bool convoyCarriesAmountInOneGo(long long truckCount, GoodAmount amount, long long goodId)
	{
		if (truckCount <= 0 || amount <= 0.f)
		{
			return false;
		}

		return truckCount * truckloadForGood(goodId) >= amount;
	}

	bool convoyIsUnderway(const Convoy& convoy)
	{
		return convoy.motion.route.size() >= 2;
	}

	bool convoyIsStanding(const Convoy& convoy)
	{
		return convoy.escortFleetId != 0;
	}

	bool standingConvoyUsesMilitaryHaulPath(const Convoy& convoy)
	{
		return convoyIsStanding(convoy) && convoy.desiredCivilianTruckCount == 0 &&
			(convoy.desiredMilitaryTruckCount > 0 || convoy.desiredTenderCount > 0);
	}

	long long standingDesiredTruckCount(const Convoy& convoy)
	{
		return convoy.desiredCivilianTruckCount + convoy.desiredMilitaryTruckCount;
	}

	long long nextConvoyId(const WorldData& world)
	{
		long long nextId = 1;
		for (const auto& convoy : world.convoys)
		{
			if (convoy.id >= nextId)
			{
				nextId = convoy.id + 1;
			}
		}

		return nextId;
	}

	long long fleetRallySystemId(const Fleet& fleet)
	{
		if (fleet.motion.route.size() >= 2)
		{
			return fleet.motion.route.back();
		}

		return fleet.systemId;
	}

	long long standingRallySystemId(const WorldData& world, const Convoy& convoy)
	{
		const Fleet* fleet = world.fleetById(convoy.escortFleetId);
		if (fleet != nullptr)
		{
			return fleetRallySystemId(*fleet);
		}

		return convoy.loadSystemId;
	}

	bool pathTruckToSystem(WorldData& world, Truck& truck, long long systemId)
	{
		const long long here = truckSystemId(truck);
		if (here == 0 || world.systemById(systemId) == nullptr)
		{
			return false;
		}

		if (here == systemId)
		{
			parkTruckAt(truck, systemId);
			return true;
		}

		if (!truck.motion.route.empty() && truck.motion.route.back() == systemId)
		{
			return true;
		}

		bool seesEscort = false;
		bool standingMilitaryPath = false;
		if (const Convoy* convoy = world.convoyById(truck.convoyId))
		{
			seesEscort = convoyIsStanding(*convoy);
			standingMilitaryPath = standingConvoyUsesMilitaryHaulPath(*convoy);
		}

		const bool isMilitary =
			truckUsesMilitaryHaulPath(world, truck) &&
			(!seesEscort || standingMilitaryPath ||
				truckHasRole(world, truck, HullRole::Tender));
		const std::vector<long long> path =
			haulLanePath(world, here, systemId, truck.empireId, seesEscort, isMilitary);
		if (path.size() < 2)
		{
			return false;
		}

		truck.motion.route = path;
		truck.motion.hop = 0;
		truck.motion.progress = 0.0f;
		truck.motion.direction = 1;
		return true;
	}

	bool convoyJobGroupIsMilitary(const WorldData& world, const Convoy& convoy)
	{
		if (convoyIsStanding(convoy))
		{
			return false;
		}

		bool any = false;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId != convoy.id)
			{
				continue;
			}

			if (!truckUsesMilitaryHaulPath(world, truck))
			{
				return false;
			}

			any = true;
		}

		return any;
	}

	Convoy* openConvoyForHaulJob(WorldData& world, long long haulJobId, bool isMilitary)
	{
		for (auto& convoy : world.convoys)
		{
			if (convoyIsStanding(convoy))
			{
				continue;
			}

			if (convoy.haulJobId != haulJobId || convoyIsUnderway(convoy))
			{
				continue;
			}

			if (countTrucksInConvoy(world, convoy.id) == 0)
			{
				continue;
			}

			if (convoyJobGroupIsMilitary(world, convoy) != isMilitary)
			{
				continue;
			}

			return &convoy;
		}

		return nullptr;
	}

	long long countTrucksInConvoy(const WorldData& world, long long convoyId)
	{
		long long count = 0;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId == convoyId)
			{
				++count;
			}
		}

		return count;
	}

	long long countMilitaryTrucksInConvoy(const WorldData& world, long long convoyId)
	{
		long long count = 0;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId == convoyId && truckHasRole(world, truck, HullRole::MilitaryCargo))
			{
				++count;
			}
		}

		return count;
	}

	long long countCivilianTrucksInConvoy(const WorldData& world, long long convoyId)
	{
		long long count = 0;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId == convoyId && truckHasRole(world, truck, HullRole::CivilianCargo))
			{
				++count;
			}
		}

		return count;
	}

	long long countTendersInConvoy(const WorldData& world, long long convoyId)
	{
		long long count = 0;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId == convoyId && truckHasRole(world, truck, HullRole::Tender))
			{
				++count;
			}
		}

		return count;
	}

	long long countCargoHaulTrucksInConvoy(const WorldData& world, long long convoyId)
	{
		long long count = 0;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId == convoyId && truckHasCargoHaulRole(world, truck))
			{
				++count;
			}
		}

		return count;
	}

	bool standingConvoyHasJoinSlotForTruck(
		const WorldData& world,
		const Convoy& convoy,
		const Truck& truck)
	{
		if (!convoyIsStanding(convoy))
		{
			return false;
		}

		if (truckHasRole(world, truck, HullRole::Tender) &&
			countTendersInConvoy(world, convoy.id) < convoy.desiredTenderCount)
		{
			return true;
		}

		if (truckHasRole(world, truck, HullRole::MilitaryCargo) &&
			countMilitaryTrucksInConvoy(world, convoy.id) < convoy.desiredMilitaryTruckCount)
		{
			return true;
		}

		if (truckHasRole(world, truck, HullRole::CivilianCargo) &&
			countCivilianTrucksInConvoy(world, convoy.id) < convoy.desiredCivilianTruckCount)
		{
			return true;
		}

		return false;
	}

	long long countTrucksInConvoyPresent(const WorldData& world, const Convoy& convoy)
	{
		if (convoyIsUnderway(convoy))
		{
			return countTrucksInConvoy(world, convoy.id);
		}

		const long long presentAt = convoyIsStanding(convoy) && convoy.haulJobId == 0
			? standingRallySystemId(world, convoy)
			: convoy.loadSystemId;
		long long count = 0;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId == convoy.id && isTruckAtSystem(truck, presentAt))
			{
				++count;
			}
		}

		return count;
	}

	long long emptyTrucksInConvoy(const WorldData& world, long long convoyId)
	{
		long long count = 0;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId == convoyId && truck.cargoAmount == 0)
			{
				++count;
			}
		}

		return count;
	}

	bool haulWaveHasStockpileForAnotherTruck(
		const WorldData& world,
		long long loadSystemId,
		long long goodId,
		long long assemblingConvoyId)
	{
		const StarSystem* supply = world.systemById(loadSystemId);
		if (supply == nullptr)
		{
			return false;
		}

		const long long truckload = truckloadForGood(goodId);
		long long reserved = 0;
		for (const auto& truck : world.trucks)
		{
			if (truck.cargoAmount != 0 || truck.haulJobId == 0)
			{
				continue;
			}

			const HaulJob* job = world.haulJobById(truck.haulJobId);
			if (job == nullptr || job->goodId != goodId)
			{
				continue;
			}

			const Convoy* convoy = world.convoyById(truck.convoyId);
			if (convoy == nullptr || convoyIsUnderway(*convoy) ||
				convoy->loadSystemId != loadSystemId)
			{
				continue;
			}

			reserved += truckload;
		}

		return getStockpileAmount(*supply, goodId) >= reserved + truckload;
	}

	GoodAmount convoyCargoOrReservedAmount(const WorldData& world, const Convoy& convoy)
	{
		GoodAmount amount = 0.f;
		for (const auto& truck : world.trucks)
		{
			if (truck.convoyId != convoy.id)
			{
				continue;
			}

			if (truck.cargoAmount > 0)
			{
				amount += truck.cargoAmount;
			}
			else
			{
				const HaulJob* job = world.haulJobById(convoy.haulJobId);
				if (job != nullptr)
				{
					amount += truckloadForGood(job->goodId);
				}
			}
		}

		return amount;
	}

	void removeConvoyIfNoMembers(WorldData& world, long long convoyId)
	{
		if (convoyId == 0 || countTrucksInConvoy(world, convoyId) > 0)
		{
			return;
		}

		const Convoy* convoy = world.convoyById(convoyId);
		if (convoy != nullptr && convoyIsStanding(*convoy))
		{
			return;
		}

		world.convoys.erase(
			std::remove_if(
				world.convoys.begin(),
				world.convoys.end(),
				[convoyId](const Convoy& convoy) { return convoy.id == convoyId; }),
			world.convoys.end());
	}

	long long systemOwnershipScore(const WorldData& world, long long systemId)
	{
		long long score = 0;
		for (const auto& mine : world.mines)
		{
			if (mine.systemId == systemId)
			{
				++score;
			}
		}

		for (const auto& factory : world.factories)
		{
			if (factory.systemId == systemId)
			{
				++score;
			}
		}

		bool population = false;
		for (const auto& yard : world.yards)
		{
			if (yard.systemId != systemId)
			{
				continue;
			}

			++score;
			if (!yard.populationConsumePerTick.empty())
			{
				population = true;
			}
		}

		if (population)
		{
			++score;
		}

		return score;
	}

	SystemOwnershipType systemOwnershipType(const WorldData& world, long long systemId)
	{
		for (const auto& yard : world.yards)
		{
			if (yard.systemId == systemId && !yard.populationConsumePerTick.empty())
			{
				return SystemOwnershipType::Populated;
			}
		}

		for (const auto& factory : world.factories)
		{
			if (factory.systemId == systemId)
			{
				return SystemOwnershipType::Industrial;
			}
		}

		for (const auto& yard : world.yards)
		{
			if (yard.systemId == systemId)
			{
				return SystemOwnershipType::Industrial;
			}
		}

		for (const auto& mine : world.mines)
		{
			if (mine.systemId == systemId)
			{
				return SystemOwnershipType::Extraction;
			}
		}

		return SystemOwnershipType::Empty;
	}

	const char* systemOwnershipTypeName(SystemOwnershipType type)
	{
		switch (type)
		{
		case SystemOwnershipType::Populated:
			return "populated";
		case SystemOwnershipType::Industrial:
			return "industrial";
		case SystemOwnershipType::Extraction:
			return "extraction";
		case SystemOwnershipType::Empty:
			return "empty";
		}

		return "unknown";
	}

	long long systemPresenceScore(const WorldData& world, long long systemId)
	{
		long long score = 0;
		for (const auto& fleet : world.fleets)
		{
			if (fleet.systemId == systemId && fleetIsParked(fleet) && fleetShipCount(fleet) > 0)
			{
				++score;
			}
		}

		return score;
	}

	bool systemHasParkedHostileFleet(const WorldData& world, long long systemId, long long viewerEmpireId)
	{
		for (const auto& fleet : world.fleets)
		{
			if (fleet.systemId != systemId || fleet.empireId == viewerEmpireId)
			{
				continue;
			}

			if (fleetIsParked(fleet) && fleetShipCount(fleet) > 0)
			{
				return true;
			}
		}

		return false;
	}

	bool systemIsAdjacentToParkedHostileFleet(
		const WorldData& world,
		long long systemId,
		long long viewerEmpireId)
	{
		for (const auto& lane : world.hyperlanes)
		{
			if (lane.a == systemId && systemHasParkedHostileFleet(world, lane.b, viewerEmpireId))
			{
				return true;
			}

			if (lane.b == systemId && systemHasParkedHostileFleet(world, lane.a, viewerEmpireId))
			{
				return true;
			}
		}

		return false;
	}

	bool hazardEventIsHot(long long eventTick, long long worldTick)
	{
		return worldTick - eventTick < CivilianHazardFadeTicks;
	}

	bool systemHasHazardHeat(const WorldData& world, long long systemId, long long viewerEmpireId)
	{
		for (const auto& report : world.battleReports)
		{
			if (report.systemId == systemId && hazardEventIsHot(report.tick, world.tick))
			{
				return true;
			}
		}

		for (const auto& loss : world.civilianLosses)
		{
			if (loss.systemId == systemId && loss.victimEmpireId == viewerEmpireId &&
				hazardEventIsHot(loss.tick, world.tick))
			{
				return true;
			}
		}

		for (const auto& convoy : world.convoys)
		{
			if (convoy.empireId != viewerEmpireId || !convoyIsUnderway(convoy))
			{
				continue;
			}

			if (convoy.expectedArrivalTick <= 0 || world.tick <= convoy.expectedArrivalTick)
			{
				continue;
			}

			if (convoy.motion.hop < 0)
			{
				continue;
			}

			for (int i = convoy.motion.hop; i < static_cast<int>(convoy.motion.route.size()); ++i)
			{
				if (convoy.motion.route[static_cast<size_t>(i)] == systemId)
				{
					return true;
				}
			}
		}

		return false;
	}

	bool systemIsAdjacentToHazardHeat(
		const WorldData& world,
		long long systemId,
		long long viewerEmpireId)
	{
		for (const auto& lane : world.hyperlanes)
		{
			if (lane.a == systemId && systemHasHazardHeat(world, lane.b, viewerEmpireId))
			{
				return true;
			}

			if (lane.b == systemId && systemHasHazardHeat(world, lane.a, viewerEmpireId))
			{
				return true;
			}
		}

		return false;
	}

	bool empiresAreAtWar(const WorldData& world, long long empireA, long long empireB)
	{
		if (empireA == 0 || empireB == 0 || empireA == empireB)
		{
			return false;
		}

		if (empireA > empireB)
		{
			std::swap(empireA, empireB);
		}

		for (const auto& row : world.empireWars)
		{
			if (row.empireA == empireA && row.empireB == empireB)
			{
				return row.atWar;
			}
		}

		return false;
	}

	void declareWarBetweenEmpires(WorldData& world, long long empireA, long long empireB)
	{
		if (empireA == 0 || empireB == 0 || empireA == empireB)
		{
			return;
		}

		if (empireA > empireB)
		{
			std::swap(empireA, empireB);
		}

		for (auto& row : world.empireWars)
		{
			if (row.empireA == empireA && row.empireB == empireB)
			{
				row.atWar = true;
				return;
			}
		}

		world.empireWars.push_back(EmpireWar{ .empireA = empireA, .empireB = empireB, .atWar = true });
	}

	bool systemClaimBlocksCivilian(
		const WorldData& world,
		long long systemId,
		long long viewerEmpireId)
	{
		const StarSystem* system = world.systemById(systemId);
		if (system == nullptr || system->empireId == 0 || system->empireId == viewerEmpireId)
		{
			return false;
		}

		return empiresAreAtWar(world, viewerEmpireId, system->empireId);
	}

	bool civilianHaulIsForbidden(
		const WorldData& world,
		long long empireId,
		long long forbiddenEmpireId)
	{
		if (empireId == 0 || forbiddenEmpireId == 0 || empireId == forbiddenEmpireId)
		{
			return false;
		}

		for (const auto& row : world.civilianHaulForbids)
		{
			if (row.empireId == empireId && row.forbiddenEmpireId == forbiddenEmpireId)
			{
				return true;
			}
		}

		return false;
	}

	void setCivilianHaulForbidden(
		WorldData& world,
		long long empireId,
		long long forbiddenEmpireId,
		bool forbidden)
	{
		if (empireId == 0 || forbiddenEmpireId == 0 || empireId == forbiddenEmpireId)
		{
			return;
		}

		if (!forbidden)
		{
			world.civilianHaulForbids.erase(
				std::remove_if(
					world.civilianHaulForbids.begin(),
					world.civilianHaulForbids.end(),
					[empireId, forbiddenEmpireId](const CivilianHaulForbid& row)
					{
						return row.empireId == empireId && row.forbiddenEmpireId == forbiddenEmpireId;
					}),
				world.civilianHaulForbids.end());
			return;
		}

		if (civilianHaulIsForbidden(world, empireId, forbiddenEmpireId))
		{
			return;
		}

		world.civilianHaulForbids.push_back(CivilianHaulForbid{
			.empireId = empireId,
			.forbiddenEmpireId = forbiddenEmpireId });
	}

	const Yard* yardAtSystem(const WorldData& world, long long systemId)
	{
		for (const auto& yard : world.yards)
		{
			if (yard.systemId == systemId)
			{
				return &yard;
			}
		}

		return nullptr;
	}

	Yard* yardAtSystem(WorldData& world, long long systemId)
	{
		return const_cast<Yard*>(yardAtSystem(static_cast<const WorldData&>(world), systemId));
	}

	long long nextYardSlipId(const WorldData& world)
	{
		long long nextId = 1;
		for (const auto& yard : world.yards)
		{
			for (const auto& slip : yard.slips)
			{
				if (slip.id >= nextId)
				{
					nextId = slip.id + 1;
				}
			}
		}

		return nextId;
	}

	long long countIdleYardSlips(const Yard& yard)
	{
		long long count = 0;
		for (const auto& slip : yard.slips)
		{
			if (slip.remainingBuildTicks == 0 && slip.hullClassId == 0)
			{
				++count;
			}
		}

		return count;
	}

	bool shipClassIsTruckOfEmpire(const ShipClass& shipClass, long long empireId)
	{
		return shipClass.empireId == empireId &&
			(shipClassHasRole(shipClass, HullRole::CivilianCargo) ||
				shipClassHasRole(shipClass, HullRole::MilitaryCargo));
	}

	bool shipClassIsWarshipOfEmpire(const ShipClass& shipClass, long long empireId)
	{
		return shipClass.empireId == empireId && shipClassHasRole(shipClass, HullRole::Warship);
	}

	GoodAmount hullSteelPerTick(const ShipClass& shipClass)
	{
		if (shipClass.buildTicks <= 0)
		{
			return 0.f;
		}

		return shipClass.steelCost / static_cast<float>(shipClass.buildTicks);
	}

	GoodAmount refitSteelPerTick()
	{
		if (RefitHullBuildTicks <= 0)
		{
			return 0.f;
		}

		return RefitHullSteelCost / static_cast<float>(RefitHullBuildTicks);
	}

	long long nextTruckId(const WorldData& world)
	{
		long long nextId = 1;
		for (const auto& truck : world.trucks)
		{
			if (truck.id >= nextId)
			{
				nextId = truck.id + 1;
			}
		}

		return nextId;
	}

	bool yardSlipIsAvailableForHull(const YardSlip& slip)
	{
		return slip.remainingBuildTicks == 0 && slip.hullClassId == 0;
	}

	void clearYardSlipHullJob(YardSlip& slip)
	{
		slip.hullClassId = 0;
		slip.remainingHullBuildTicks = 0;
		slip.refitHullId = 0;
		slip.refitFromClassId = 0;
		slip.preferredFleetId = 0;
	}

	YardSlip* yardSlipById(Yard& yard, long long slipId)
	{
		for (auto& slip : yard.slips)
		{
			if (slip.id == slipId)
			{
				return &slip;
			}
		}

		return nullptr;
	}

	Fleet* parkedFleetAtYardForRefitReturn(
		WorldData& world,
		const Yard& yard,
		long long preferredFleetId)
	{
		Fleet* preferred = world.fleetById(preferredFleetId);
		if (preferred != nullptr && preferred->empireId == yard.empireId &&
			preferred->systemId == yard.systemId && fleetIsParked(*preferred))
		{
			return preferred;
		}

		Fleet* best = nullptr;
		for (auto& fleet : world.fleets)
		{
			if (fleet.empireId != yard.empireId || fleet.systemId != yard.systemId ||
				!fleetIsParked(fleet))
			{
				continue;
			}

			if (best == nullptr || fleet.id < best->id)
			{
				best = &fleet;
			}
		}

		return best;
	}

	Fleet& fleetAtYardForCompletedRefit(WorldData& world, Yard& yard, long long preferredFleetId)
	{
		Fleet* parked = parkedFleetAtYardForRefitReturn(world, yard, preferredFleetId);
		if (parked != nullptr)
		{
			return *parked;
		}

		const StarSystem* system = world.systemById(yard.systemId);
		const std::string systemName = system != nullptr ? system->name : "yard";
		world.fleets.push_back(Fleet{
			.id = nextFleetId(world),
			.empireId = yard.empireId,
			.name = systemName + " yard",
			.systemId = yard.systemId,
			.motion = { .speed = 50.0f } });
		return world.fleets.back();
	}

	void assignYardHullJobsToAvailableSlips(WorldData& world, Yard& yard)
	{
		std::vector<long long> availableIds;
		for (const auto& slip : yard.slips)
		{
			if (yardSlipIsAvailableForHull(slip))
			{
				availableIds.push_back(slip.id);
			}
		}

		std::sort(availableIds.begin(), availableIds.end());
		for (const long long slipId : availableIds)
		{
			YardSlip* slip = yardSlipById(yard, slipId);
			if (slip == nullptr)
			{
				continue;
			}

			if (!yard.hullRefitQueue.empty())
			{
				const YardRefitJob job = yard.hullRefitQueue.front();
				yard.hullRefitQueue.erase(yard.hullRefitQueue.begin());
				slip->hullClassId = job.toClassId;
				slip->remainingHullBuildTicks = RefitHullBuildTicks;
				slip->refitHullId = job.hullId;
				slip->refitFromClassId = job.fromClassId;
				slip->preferredFleetId = job.preferredFleetId;
				continue;
			}

			const ShipClass* shipClass = nullptr;
			while (!yard.hullBuildQueue.empty() && shipClass == nullptr)
			{
				const long long classId = yard.hullBuildQueue.front();
				shipClass = world.shipClassById(classId);
				if (shipClass == nullptr || shipClass->buildTicks <= 0)
				{
					yard.hullBuildQueue.erase(yard.hullBuildQueue.begin());
					shipClass = nullptr;
				}
			}

			if (shipClass == nullptr)
			{
				break;
			}

			yard.hullBuildQueue.erase(yard.hullBuildQueue.begin());
			slip->hullClassId = shipClass->id;
			slip->remainingHullBuildTicks = shipClass->buildTicks;
		}
	}

	void spawnParkedTruckFromClass(WorldData& world, const Yard& yard, const ShipClass& shipClass)
	{
		Truck truck;
		truck.id = nextTruckId(world);
		truck.empireId = yard.empireId;
		truck.classId = shipClass.id;
		truck.motion.speed = 25.0f;
		parkTruckAt(truck, yard.systemId);
		world.trucks.push_back(truck);
	}

	void applyYardHullConstruction(WorldData& world)
	{
		for (auto& yard : world.yards)
		{
			assignYardHullJobsToAvailableSlips(world, yard);

			std::vector<long long> busyIds;
			for (const auto& slip : yard.slips)
			{
				if (slip.hullClassId != 0)
				{
					busyIds.push_back(slip.id);
				}
			}

			std::sort(busyIds.begin(), busyIds.end());
			for (const long long slipId : busyIds)
			{
				YardSlip* slip = yardSlipById(yard, slipId);
				if (slip == nullptr)
				{
					continue;
				}

				const bool isRefit = slip->refitHullId != 0;
				const ShipClass* shipClass = world.shipClassById(slip->hullClassId);
				if (shipClass == nullptr)
				{
					clearYardSlipHullJob(*slip);
					continue;
				}

				const long long slice = isRefit ? refitSteelPerTick() : hullSteelPerTick(*shipClass);
				if (slice <= 0)
				{
					continue;
				}

				if (stockpileAvailableForYardConsume(world, yard.systemId, SteelGoodId) < slice)
				{
					continue;
				}

				if (!useFromStockpile(world, yard.systemId, SteelGoodId, slice))
				{
					continue;
				}

				slip->remainingHullBuildTicks -= 1;
				if (slip->remainingHullBuildTicks > 0)
				{
					continue;
				}

				if (isRefit)
				{
					Fleet& fleet = fleetAtYardForCompletedRefit(world, yard, slip->preferredFleetId);
					ensureFleetClassDesiredCargo(fleet, slip->hullClassId);
					fleet.hulls.push_back(FleetHull{
						.id = slip->refitHullId,
						.classId = slip->hullClassId });
					syncFleetCompositionFromHulls(fleet);
					clearYardSlipHullJob(*slip);
					continue;
				}

				spawnParkedTruckFromClass(world, yard, *shipClass);
				clearYardSlipHullJob(*slip);
			}
		}
	}

	void applyYardMagazineRestock(WorldData& world)
	{
		struct ParkedHullRef
		{
			long long fleetId{ 0 };
			long long hullId{ 0 };
		};

		for (const auto& yard : world.yards)
		{
			std::vector<ParkedHullRef> hulls;
			for (const auto& fleet : world.fleets)
			{
				if (fleet.empireId != yard.empireId || fleet.systemId != yard.systemId ||
					!fleetIsParked(fleet))
				{
					continue;
				}

				for (const auto& hull : fleet.hulls)
				{
					hulls.push_back(ParkedHullRef{ .fleetId = fleet.id, .hullId = hull.id });
				}
			}

			std::sort(hulls.begin(), hulls.end(), [](const ParkedHullRef& a, const ParkedHullRef& b) {
				return a.hullId < b.hullId;
			});

			for (const auto& ref : hulls)
			{
				Fleet* fleet = world.fleetById(ref.fleetId);
				if (fleet == nullptr)
				{
					continue;
				}

				FleetHull* hull = nullptr;
				for (auto& candidate : fleet->hulls)
				{
					if (candidate.id == ref.hullId)
					{
						hull = &candidate;
						break;
					}
				}

				if (hull == nullptr)
				{
					continue;
				}

				const ShipClass* shipClass = world.shipClassById(hull->classId);
				if (shipClass == nullptr || shipClass->magazineSize <= 0 ||
					magazineUsed(hull->magazine) >= static_cast<GoodAmount>(shipClass->magazineSize))
				{
					continue;
				}

				std::vector<GoodQuantity> available;
				for (const long long goodId : { MissileGoodId, MissileLongRangeGoodId })
				{
					available.push_back(GoodQuantity{
						.goodId = goodId,
						.amount = stockpileAvailableForYardConsume(world, yard.systemId, goodId) });
				}

				const long long nextGood = nextDesiredCargoGoodId(
					desiredCargoForFleetClass(*fleet, hull->classId),
					hull->magazine,
					static_cast<GoodAmount>(shipClass->magazineSize),
					available);
				if (nextGood == 0)
				{
					continue;
				}

				if (!useFromStockpile(world, yard.systemId, nextGood, 1))
				{
					continue;
				}

				addMagazineGood(hull->magazine, nextGood, 1.f);
			}
		}
	}

	bool tenderIsParkedAtSameEmpireYard(const WorldData& world, const Truck& truck)
	{
		if (!truckHasRole(world, truck, HullRole::Tender) || truck.motion.progress > 0.0f)
		{
			return false;
		}

		const long long systemId = truckSystemId(truck);
		const Yard* yard = yardAtSystem(world, systemId);
		return yard != nullptr && yard->empireId == truck.empireId && isTruckAtSystem(truck, systemId);
	}

	void applyTenderLoadFromYard(WorldData& world)
	{
		std::vector<long long> tenderIds;
		for (const auto& truck : world.trucks)
		{
			if (!tenderIsParkedAtSameEmpireYard(world, truck))
			{
				continue;
			}

			const Convoy* convoy = world.convoyById(truck.convoyId);
			if (convoy == nullptr || !convoyIsStanding(*convoy))
			{
				continue;
			}

			if (truck.cargoAmount >= TruckCapacity_Missiles)
			{
				continue;
			}

			tenderIds.push_back(truck.id);
		}

		std::sort(tenderIds.begin(), tenderIds.end());
		for (const long long tenderId : tenderIds)
		{
			Truck* tender = world.truckById(tenderId);
			if (tender == nullptr)
			{
				continue;
			}

			const Convoy* convoy = world.convoyById(tender->convoyId);
			if (convoy == nullptr)
			{
				continue;
			}

			const long long systemId = truckSystemId(*tender);
			std::vector<GoodQuantity> available;
			for (const long long goodId : { MissileGoodId, MissileLongRangeGoodId })
			{
				available.push_back(GoodQuantity{
					.goodId = goodId,
					.amount = stockpileAvailableForYardConsume(world, systemId, goodId) });
			}

			const long long nextGood = nextDesiredCargoGoodId(
				convoy->holdCargo,
				truckCargoAsQuantities(*tender),
				TruckCapacity_Missiles,
				available);
			if (nextGood == 0)
			{
				continue;
			}

			if (tender->cargoAmount > 0.f && tender->cargoGoodId != nextGood)
			{
				continue;
			}

			if (!useFromStockpile(world, systemId, nextGood, 1))
			{
				continue;
			}

			tender->cargoGoodId = nextGood;
			tender->cargoAmount += 1;
		}
	}

	void applyTenderUnloadIntoMagazines(WorldData& world)
	{
		struct MagazineHullRef
		{
			long long fleetId{ 0 };
			long long hullId{ 0 };
			long long empireId{ 0 };
			long long systemId{ 0 };
		};

		std::vector<MagazineHullRef> hulls;
		for (const auto& fleet : world.fleets)
		{
			if (!fleetIsParked(fleet))
			{
				continue;
			}

			for (const auto& hull : fleet.hulls)
			{
				const ShipClass* shipClass = world.shipClassById(hull.classId);
				if (shipClass == nullptr || !shipClassHasRole(*shipClass, HullRole::Warship) ||
					shipClass->magazineSize <= 0 ||
					magazineUsed(hull.magazine) >= static_cast<GoodAmount>(shipClass->magazineSize))
				{
					continue;
				}

				hulls.push_back(MagazineHullRef{
					.fleetId = fleet.id,
					.hullId = hull.id,
					.empireId = fleet.empireId,
					.systemId = fleet.systemId });
			}
		}

		std::sort(hulls.begin(), hulls.end(), [](const MagazineHullRef& a, const MagazineHullRef& b) {
			return a.hullId < b.hullId;
		});

		for (const auto& ref : hulls)
		{
			Truck* donor = nullptr;
			for (auto& truck : world.trucks)
			{
				if (!truckHasRole(world, truck, HullRole::Tender) || truck.motion.progress > 0.0f ||
					truck.cargoAmount <= 0 || !isCombatMissileGood(truck.cargoGoodId) ||
					truck.empireId != ref.empireId)
				{
					continue;
				}

				if (tenderIsParkedAtSameEmpireYard(world, truck))
				{
					continue;
				}

				const Convoy* convoy = world.convoyById(truck.convoyId);
				if (convoy == nullptr || !convoyIsStanding(*convoy))
				{
					continue;
				}

				const Fleet* escort = world.fleetById(convoy->escortFleetId);
				if (escort == nullptr || !fleetIsParked(*escort) || escort->systemId != ref.systemId)
				{
					continue;
				}

				if (!isTruckAtSystem(truck, ref.systemId))
				{
					continue;
				}

				if (donor == nullptr || truck.id < donor->id)
				{
					donor = &truck;
				}
			}

			if (donor == nullptr)
			{
				continue;
			}

			Fleet* fleet = world.fleetById(ref.fleetId);
			if (fleet == nullptr)
			{
				continue;
			}

			for (auto& hull : fleet->hulls)
			{
				if (hull.id != ref.hullId)
				{
					continue;
				}

				const ShipClass* shipClass = world.shipClassById(hull.classId);
				if (shipClass == nullptr ||
					!hullWantsCombatMissileGood(
						desiredCargoForFleetClass(*fleet, hull.classId),
						hull,
						static_cast<GoodAmount>(shipClass->magazineSize),
						donor->cargoGoodId))
				{
					continue;
				}

				addMagazineGood(hull.magazine, donor->cargoGoodId, 1.f);
				donor->cargoAmount -= 1;
				if (donor->cargoAmount == 0)
				{
					donor->cargoGoodId = 0;
				}

				break;
			}
		}
	}

	void applyYardSlipConstruction(WorldData& world)
	{
		for (auto& yard : world.yards)
		{
			std::vector<long long> constructing;
			for (const auto& slip : yard.slips)
			{
				if (slip.remainingBuildTicks > 0)
				{
					constructing.push_back(slip.id);
				}
			}

			if (constructing.empty())
			{
				yard.nextSlipConstructionId = 0;
				continue;
			}

			std::sort(constructing.begin(), constructing.end());
			long long chosenId = constructing.front();
			for (const long long id : constructing)
			{
				if (id >= yard.nextSlipConstructionId)
				{
					chosenId = id;
					break;
				}
			}

			for (auto& slip : yard.slips)
			{
				if (slip.id == chosenId)
				{
					slip.remainingBuildTicks -= 1;
					break;
				}
			}

			constructing.clear();
			for (const auto& slip : yard.slips)
			{
				if (slip.remainingBuildTicks > 0)
				{
					constructing.push_back(slip.id);
				}
			}

			if (constructing.empty())
			{
				yard.nextSlipConstructionId = 0;
				continue;
			}

			std::sort(constructing.begin(), constructing.end());
			long long nextId = constructing.front();
			for (const long long id : constructing)
			{
				if (id > chosenId)
				{
					nextId = id;
					break;
				}
			}

			yard.nextSlipConstructionId = nextId;
		}
	}

}
