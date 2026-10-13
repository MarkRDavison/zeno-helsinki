#include <World/ShipDesign.hpp>

namespace slr
{

	const HullSizeSpec& hullSizeSpec(HullSize size)
	{
		static constexpr HullSizeSpec small{
			.size = HullSize::Small,
			.slots = 8,
			.steelCost = 2000.f,
			.buildTicks = 20,
			.maxHull = 28,
			.maxShields = 20 };
		static constexpr HullSizeSpec medium{
			.size = HullSize::Medium,
			.slots = 12,
			.steelCost = 4000.f,
			.buildTicks = 32,
			.maxHull = 50,
			.maxShields = 36 };
		static constexpr HullSizeSpec large{
			.size = HullSize::Large,
			.slots = 18,
			.steelCost = 7000.f,
			.buildTicks = 48,
			.maxHull = 80,
			.maxShields = 56 };
		if (size == HullSize::Medium)
		{
			return medium;
		}

		if (size == HullSize::Large)
		{
			return large;
		}

		return small;
	}

	const char* hullSizeName(HullSize size)
	{
		if (size == HullSize::Medium)
		{
			return "Medium";
		}

		if (size == HullSize::Large)
		{
			return "Large";
		}

		return "Small";
	}

	const char* shipClassDraftFaultName(ShipClassDraftFault fault)
	{
		switch (fault)
		{
		case ShipClassDraftFault::EmptyName:
			return "Name is empty";
		case ShipClassDraftFault::NoEngine:
			return "Need at least one engine";
		case ShipClassDraftFault::OverSlots:
			return "Over hull slots";
		case ShipClassDraftFault::PowerNotPositive:
			return "Power is not positive";
		case ShipClassDraftFault::None:
			return "";
		}

		return "";
	}

	long long shipDesignUsedSlots(const ShipDesignPartCounts& parts)
	{
		return parts.reactorCount + parts.engineCount + parts.laserCount + parts.missileTubeCount +
			parts.cargoCount;
	}

	long long shipDesignNetPower(const ShipDesignPartCounts& parts)
	{
		return parts.reactorCount * ShipDesignReactorPower -
			parts.engineCount * ShipDesignEnginePowerDraw -
			parts.laserCount * ShipDesignLaserPowerDraw -
			parts.missileTubeCount * ShipDesignMissileTubePowerDraw;
	}

	ShipClassDraftFault validateShipClassDraft(const ShipClassDraft& draft)
	{
		if (draft.name.empty())
		{
			return ShipClassDraftFault::EmptyName;
		}

		if (draft.parts.engineCount < 1)
		{
			return ShipClassDraftFault::NoEngine;
		}

		if (draft.parts.reactorCount < 0 || draft.parts.engineCount < 0 ||
			draft.parts.laserCount < 0 || draft.parts.missileTubeCount < 0 ||
			draft.parts.cargoCount < 0)
		{
			return ShipClassDraftFault::OverSlots;
		}

		if (shipDesignUsedSlots(draft.parts) > hullSizeSpec(draft.hullSize).slots)
		{
			return ShipClassDraftFault::OverSlots;
		}

		if (shipDesignNetPower(draft.parts) <= 0)
		{
			return ShipClassDraftFault::PowerNotPositive;
		}

		return ShipClassDraftFault::None;
	}

	ShipClass makeWarshipClassFromDraft(
		long long id,
		long long empireId,
		const ShipClassDraft& draft)
	{
		const HullSizeSpec& hull = hullSizeSpec(draft.hullSize);
		const long long partCount = shipDesignUsedSlots(draft.parts);
		ShipClass shipClass;
		shipClass.id = id;
		shipClass.empireId = empireId;
		shipClass.name = draft.name;
		shipClass.range = WarshipDesignRange;
		shipClass.accuracy = WarshipDesignAccuracy;
		shipClass.laserDamage = draft.parts.laserCount * ShipDesignLaserDamage;
		shipClass.maxShields = hull.maxShields;
		shipClass.maxHull = hull.maxHull;
		shipClass.closeRate = static_cast<float>(draft.parts.engineCount * ShipDesignEngineCloseRate);
		shipClass.hullRoles = hullRoleBit(HullRole::Warship);
		shipClass.hullSize = draft.hullSize;
		shipClass.steelCost = hull.steelCost +
			static_cast<GoodAmount>(partCount * ShipDesignSteelPerPart);
		shipClass.buildTicks = hull.buildTicks;
		shipClass.missileTubes = draft.parts.missileTubeCount;
		shipClass.missileReloadInnerTicks = WarshipDesignMissileReloadInnerTicks;
		shipClass.magazineSize = draft.parts.cargoCount * ShipDesignCargoMagazine;
		shipClass.missileDamage = draft.parts.missileTubeCount * ShipDesignMissileTubeDamage;
		shipClass.missileRange = WarshipDesignMissileRange;
		shipClass.missileSpeed = WarshipDesignMissileSpeed;
		shipClass.reactorCount = draft.parts.reactorCount;
		shipClass.engineCount = draft.parts.engineCount;
		shipClass.laserCount = draft.parts.laserCount;
		shipClass.missileTubeCount = draft.parts.missileTubeCount;
		shipClass.cargoCount = draft.parts.cargoCount;
		return shipClass;
	}

	long long nextShipClassId(const WorldData& world)
	{
		long long nextId = 1;
		for (const auto& shipClass : world.shipClasses)
		{
			if (shipClass.id >= nextId)
			{
				nextId = shipClass.id + 1;
			}
		}

		return nextId;
	}

}
