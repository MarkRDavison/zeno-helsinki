#include <Services/UpgradeService.hpp>
#include <Services/PrototypeService.hpp>
#include <algorithm>
#include <stdexcept>

namespace drl
{

	UpgradeService::UpgradeService(UpgradeData& upgradeData)
		: _upgradeData(upgradeData)
	{
	}

	void UpgradeService::addUpgrade(long long upgradeId, float value)
	{
		if (upgradeId != prototypeIdFromName(UpgradeRefine))
		{
			return;
		}

		_upgradeData.oreMultiplier += value;
	}

	float UpgradeService::oreMultiplier() const
	{
		return _upgradeData.oreMultiplier;
	}

	void UpgradeService::registerHud(
		const std::string& name,
		int order,
		std::string label,
		std::string description)
	{
		for (const auto& [existingName, entry] : _hud)
		{
			if (entry.order == order && existingName != name)
			{
				throw std::invalid_argument("duplicate HUD order " + std::to_string(order));
			}
		}

		_hud[name] = HudEntry{
			.order = order,
			.label = std::move(label),
			.description = std::move(description)
		};
	}

	bool UpgradeService::hasHud(const std::string& name) const
	{
		return _hud.contains(name);
	}

	int UpgradeService::getOrder(const std::string& name) const
	{
		const auto it = _hud.find(name);
		if (it == _hud.end())
		{
			throw std::out_of_range("upgrade HUD is not registered: " + name);
		}
		return it->second.order;
	}

	const std::string& UpgradeService::getLabel(const std::string& name) const
	{
		const auto it = _hud.find(name);
		if (it == _hud.end())
		{
			throw std::out_of_range("upgrade HUD is not registered: " + name);
		}
		return it->second.label;
	}

	const std::string& UpgradeService::getDescription(const std::string& name) const
	{
		const auto it = _hud.find(name);
		if (it == _hud.end())
		{
			throw std::out_of_range("upgrade HUD is not registered: " + name);
		}
		return it->second.description;
	}

	std::vector<std::string> UpgradeService::registeredHudNames() const
	{
		std::vector<std::string> names;
		names.reserve(_hud.size());
		for (const auto& [name, entry] : _hud)
		{
			names.push_back(name);
		}
		std::sort(names.begin(), names.end(), [this](const std::string& a, const std::string& b)
		{
			return getOrder(a) < getOrder(b);
		});
		return names;
	}

}
