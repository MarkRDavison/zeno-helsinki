#include <Systems/WeaponFiringSystem.hpp>

namespace hur
{

	WeaponFiringSystem::WeaponFiringSystem(
		hl::EventBus& eventBus,
		hl::Scene& scene
	) :
		_eventBus(eventBus),
		_scene(scene)
	{

	}

	void WeaponFiringSystem::update(float delta)
	{

	}

}