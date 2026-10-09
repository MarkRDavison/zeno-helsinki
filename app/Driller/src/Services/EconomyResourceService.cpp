#include <Services/EconomyResourceService.hpp>

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
		const auto it = _resources.find(name);
		if (it == _resources.end())
		{
			return 0;
		}
		return it->second.amount;
	}

	void EconomyResourceService::setMax(const std::string& name, long long maximum)
	{
		auto& entry = getOrCreate(name);
		entry.max = maximum;
		clampToMax(entry);
	}

	long long EconomyResourceService::getMax(const std::string& name) const
	{
		const auto it = _resources.find(name);
		if (it == _resources.end())
		{
			return -1;
		}
		return it->second.max;
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

}
