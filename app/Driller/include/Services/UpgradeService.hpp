#pragma once

#include <Entities/Data/UpgradeData.hpp>

namespace drl
{

	inline constexpr const char* UpgradeRefine = "Upgrade_Refine";

	class IUpgradeService
	{
	public:
		virtual ~IUpgradeService() = 0;

		virtual void addUpgrade(long long upgradeId, float value) = 0;
		virtual float oreMultiplier() const = 0;
	};

	inline IUpgradeService::~IUpgradeService() = default;

	class UpgradeService : public IUpgradeService
	{
	public:
		explicit UpgradeService(UpgradeData& upgradeData);
		~UpgradeService() override = default;

		void addUpgrade(long long upgradeId, float value) override;
		float oreMultiplier() const override;

	private:
		UpgradeData& _upgradeData;
	};

}
