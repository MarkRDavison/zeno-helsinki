#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace drl
{

	inline constexpr const char* ResourceOre = "Resource_Ore";
	inline constexpr const char* ResourceMoney = "Resource_Money";

	class IEconomyResourceService
	{
	public:
		virtual ~IEconomyResourceService() = 0;

		virtual bool exists(const std::string& name) const = 0;
		virtual void set(const std::string& name, long long amount) = 0;
		virtual long long get(const std::string& name) const = 0;
		virtual void setMax(const std::string& name, long long maximum) = 0;
		virtual long long getMax(const std::string& name) const = 0;
		virtual bool canAfford(const std::string& name, long long amount) const = 0;
		virtual bool pay(const std::string& name, long long amount) = 0;
		virtual void add(const std::string& name, long long amount) = 0;

		virtual void setHud(
			const std::string& name,
			int order,
			std::string label,
			std::string description) = 0;
		virtual bool hasHud(const std::string& name) const = 0;
		virtual int getOrder(const std::string& name) const = 0;
		virtual const std::string& getLabel(const std::string& name) const = 0;
		virtual const std::string& getDescription(const std::string& name) const = 0;
		virtual std::vector<std::string> registeredHudNames() const = 0;
	};

	inline IEconomyResourceService::~IEconomyResourceService() = default;

	class EconomyResourceService : public IEconomyResourceService
	{
	public:
		~EconomyResourceService() override = default;

		bool exists(const std::string& name) const override;
		void set(const std::string& name, long long amount) override;
		long long get(const std::string& name) const override;
		void setMax(const std::string& name, long long maximum) override;
		long long getMax(const std::string& name) const override;
		bool canAfford(const std::string& name, long long amount) const override;
		bool pay(const std::string& name, long long amount) override;
		void add(const std::string& name, long long amount) override;

		void setHud(
			const std::string& name,
			int order,
			std::string label,
			std::string description) override;
		bool hasHud(const std::string& name) const override;
		int getOrder(const std::string& name) const override;
		const std::string& getLabel(const std::string& name) const override;
		const std::string& getDescription(const std::string& name) const override;
		std::vector<std::string> registeredHudNames() const override;

	private:
		struct Entry
		{
			long long amount{ 0 };
			long long max{ -1 };
			bool hasHud{ false };
			int order{ 0 };
			std::string label;
			std::string description;
		};

		Entry& getOrCreate(const std::string& name);
		const Entry* find(const std::string& name) const;
		void clampToMax(Entry& entry) const;

		std::unordered_map<std::string, Entry> _resources;
	};

}
