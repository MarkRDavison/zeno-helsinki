#include <Services/ShuttleCreationService.hpp>

namespace drl
{

	ShuttleCreationService::ShuttleCreationService(
		ShuttleData& shuttleData,
		IShuttlePrototypeService& shuttlePrototypeService)
		: _shuttleData(shuttleData)
		, _shuttlePrototypeService(shuttlePrototypeService)
	{
	}

	bool ShuttleCreationService::createShuttle(ShuttlePrototypeId prototypeId)
	{
		if (!_shuttlePrototypeService.isPrototypeRegistered(prototypeId))
		{
			return false;
		}

		_shuttleData.shuttles.push_back(_shuttlePrototypeService.createInstance(prototypeId));
		return true;
	}

}
