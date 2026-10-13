#pragma once

#include <World/WorldData.hpp>
#include <string>

namespace slr
{

	constexpr long long ShipDesignReactorPower = 16;
	constexpr long long ShipDesignEnginePowerDraw = 4;
	constexpr long long ShipDesignLaserPowerDraw = 2;
	constexpr long long ShipDesignMissileTubePowerDraw = 2;
	constexpr long long ShipDesignEngineCloseRate = 3;
	constexpr long long ShipDesignLaserDamage = 4;
	constexpr long long ShipDesignMissileTubeDamage = 4;
	constexpr long long ShipDesignCargoMagazine = 40;
	constexpr long long ShipDesignSteelPerPart = 50;
	constexpr float WarshipDesignRange = 55.0f;
	constexpr float WarshipDesignAccuracy = 0.75f;
	constexpr long long WarshipDesignMissileReloadInnerTicks = 3;
	constexpr float WarshipDesignMissileRange = 90.0f;
	constexpr float WarshipDesignMissileSpeed = 25.0f;

	struct HullSizeSpec
	{
		HullSize size{ HullSize::Small };
		long long slots{ 0 };
		GoodAmount steelCost{ 0.f };
		long long buildTicks{ 0 };
		long long maxHull{ 0 };
		long long maxShields{ 0 };
	};

	struct ShipDesignPartCounts
	{
		long long reactorCount{ 0 };
		long long engineCount{ 0 };
		long long laserCount{ 0 };
		long long missileTubeCount{ 0 };
		long long cargoCount{ 0 };
	};

	struct ShipClassDraft
	{
		std::string name;
		HullSize hullSize{ HullSize::Small };
		ShipDesignPartCounts parts;
	};

	enum class ShipClassDraftFault
	{
		None,
		EmptyName,
		NoEngine,
		OverSlots,
		PowerNotPositive
	};

	const HullSizeSpec& hullSizeSpec(HullSize size);
	const char* hullSizeName(HullSize size);
	const char* shipClassDraftFaultName(ShipClassDraftFault fault);
	long long shipDesignUsedSlots(const ShipDesignPartCounts& parts);
	long long shipDesignNetPower(const ShipDesignPartCounts& parts);
	ShipClassDraftFault validateShipClassDraft(const ShipClassDraft& draft);
	ShipClass makeWarshipClassFromDraft(
		long long id,
		long long empireId,
		const ShipClassDraft& draft);
	long long nextShipClassId(const WorldData& world);

}
