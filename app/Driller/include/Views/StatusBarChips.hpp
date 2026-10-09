#pragma once

#include <Services/EconomyResourceService.hpp>
#include <Services/UpgradeService.hpp>
#include <format>
#include <string>
#include <string_view>
#include <vector>

namespace drl
{

	enum class StatusChipKind
	{
		Resource,
		Upgrade
	};

	struct StatusChip
	{
		StatusChipKind kind{ StatusChipKind::Resource };
		std::string name;
		int order{ 0 };
		std::string label;
		std::string description;
	};

	inline std::vector<StatusChip> collectStatusBarChips(
		const IEconomyResourceService& economy,
		const IUpgradeService& upgrades)
	{
		std::vector<StatusChip> chips;
		for (const std::string& name : economy.registeredHudNames())
		{
			chips.push_back(StatusChip{
				.kind = StatusChipKind::Resource,
				.name = name,
				.order = economy.getOrder(name),
				.label = economy.getLabel(name),
				.description = economy.getDescription(name)
			});
		}
		for (const std::string& name : upgrades.registeredHudNames())
		{
			chips.push_back(StatusChip{
				.kind = StatusChipKind::Upgrade,
				.name = name,
				.order = upgrades.getOrder(name),
				.label = upgrades.getLabel(name),
				.description = upgrades.getDescription(name)
			});
		}
		return chips;
	}

	inline std::string formatStatusValue(
		const StatusChip& chip,
		const IEconomyResourceService& economy,
		const IUpgradeService& upgrades)
	{
		if (chip.kind == StatusChipKind::Resource)
		{
			return std::to_string(economy.get(chip.name));
		}
		if (chip.name == UpgradeRefine)
		{
			return std::format("{:.3f}", upgrades.oreMultiplier());
		}
		return "0.000";
	}

	inline std::string formatStatusTooltip(
		std::string_view label,
		std::string_view value,
		std::string_view description)
	{
		return std::format("{}: {}\n{}", label, value, description);
	}

}
