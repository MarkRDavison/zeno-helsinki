#include <Services/UpgradeService.hpp>
#include <Services/PrototypeService.hpp>

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

}
