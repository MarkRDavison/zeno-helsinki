#include <Services/EconomyResourceService.hpp>
#include <algorithm>
#include <stdexcept>

namespace drl
{

	bool EconomyResourceService::exists(const std::string& name) const
	{
		return _resources.contains(name);
	}

	EconomyResourceService::Entry& EconomyResourceService::getOrCreate(const std::string& name)
	{
		return _resources[name];
	}

	const EconomyResourceService::Entry* EconomyResourceService::find(const std::string& name) const
	{
		const auto it = _resources.find(name);
		if (it == _resources.end())
		{
			return nullptr;
		}
		return &it->second;
	}

	void EconomyResourceService::clampToMax(Entry& entry) const
	{
		if (entry.max != -1 && entry.amount > entry.max)
		{
			entry.amount = entry.max;
		}
	}

	void EconomyResourceService::set(const std::string& name, long long amount)
	{
		auto& entry = getOrCreate(name);
		entry.amount = amount;
		clampToMax(entry);
	}

	long long EconomyResourceService::get(const std::string& name) const
	{
		const auto* entry = find(name);
		if (entry == nullptr)
		{
			return 0;
		}
		return entry->amount;
	}

	void EconomyResourceService::setMax(const std::string& name, long long maximum)
	{
		auto& entry = getOrCreate(name);
		entry.max = maximum;
		clampToMax(entry);
	}

	long long EconomyResourceService::getMax(const std::string& name) const
	{
		const auto* entry = find(name);
		if (entry == nullptr)
		{
			return -1;
		}
		return entry->max;
	}

	bool EconomyResourceService::canAfford(const std::string& name, long long amount) const
	{
		if (!exists(name))
		{
			return false;
		}
		return amount <= get(name);
	}

	bool EconomyResourceService::pay(const std::string& name, long long amount)
	{
		if (!canAfford(name, amount))
		{
			return false;
		}
		_resources[name].amount -= amount;
		return true;
	}

	void EconomyResourceService::add(const std::string& name, long long amount)
	{
		auto& entry = getOrCreate(name);
		entry.amount += amount;
		clampToMax(entry);
	}

	void EconomyResourceService::setHud(
		const std::string& name,
		int order,
		std::string label,
		std::string description)
	{
		for (const auto& [existingName, entry] : _resources)
		{
			if (entry.hasHud && entry.order == order && existingName != name)
			{
				throw std::invalid_argument("duplicate HUD order " + std::to_string(order));
			}
		}

		auto& entry = getOrCreate(name);
		entry.hasHud = true;
		entry.order = order;
		entry.label = std::move(label);
		entry.description = std::move(description);
	}

	bool EconomyResourceService::hasHud(const std::string& name) const
	{
		const auto* entry = find(name);
		return entry != nullptr && entry->hasHud;
	}

	int EconomyResourceService::getOrder(const std::string& name) const
	{
		const auto* entry = find(name);
		if (entry == nullptr || !entry->hasHud)
		{
			throw std::out_of_range("resource HUD is not registered: " + name);
		}
		return entry->order;
	}

	const std::string& EconomyResourceService::getLabel(const std::string& name) const
	{
		const auto* entry = find(name);
		if (entry == nullptr || !entry->hasHud)
		{
			throw std::out_of_range("resource HUD is not registered: " + name);
		}
		return entry->label;
	}

	const std::string& EconomyResourceService::getDescription(const std::string& name) const
	{
		const auto* entry = find(name);
		if (entry == nullptr || !entry->hasHud)
		{
			throw std::out_of_range("resource HUD is not registered: " + name);
		}
		return entry->description;
	}

	std::vector<std::string> EconomyResourceService::registeredHudNames() const
	{
		std::vector<std::string> names;
		for (const auto& [name, entry] : _resources)
		{
			if (entry.hasHud)
			{
				names.push_back(name);
			}
		}
		std::sort(names.begin(), names.end(), [this](const std::string& a, const std::string& b)
		{
			return getOrder(a) < getOrder(b);
		});
		return names;
	}

}
