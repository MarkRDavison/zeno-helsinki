#pragma once

#include <Entities/Data/UpgradeData.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace drl
{

	inline constexpr const char* UpgradeRefine = "Upgrade_Refine";

	class IUpgradeService
	{
	public:
		virtual ~IUpgradeService() = 0;

		virtual void addUpgrade(long long upgradeId, float value) = 0;
		virtual float oreMultiplier() const = 0;

		virtual void registerHud(
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

	inline IUpgradeService::~IUpgradeService() = default;

	class UpgradeService : public IUpgradeService
	{
	public:
		explicit UpgradeService(UpgradeData& upgradeData);
		~UpgradeService() override = default;

		void addUpgrade(long long upgradeId, float value) override;
		float oreMultiplier() const override;

		void registerHud(
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
		struct HudEntry
		{
			int order{ 0 };
			std::string label;
			std::string description;
		};

		UpgradeData& _upgradeData;
		std::unordered_map<std::string, HudEntry> _hud;
	};

}
