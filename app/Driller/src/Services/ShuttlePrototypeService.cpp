#include <Services/ShuttlePrototypeService.hpp>

namespace drl
{

	ShuttleInstance ShuttlePrototypeService::createInstanceFromPrototype(const ShuttlePrototype& prototype)
	{
		ShuttleInstance shuttle{};
		shuttle.id = allocateInstanceId();
		shuttle.prototypeId = prototypeIdFromName(prototype.name);
		shuttle.state = ShuttleState::Idle;
		shuttle.elapsed = 0.0f;
		shuttle.startingPosition = kShuttleStartingPosition;
		shuttle.surfacePosition = kShuttleSurfacePosition;
		shuttle.leavingPosition = kShuttleLeavingPosition;
		shuttle.position = shuttle.startingPosition;
		return shuttle;
	}

}
