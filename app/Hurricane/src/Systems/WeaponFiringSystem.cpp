#include <Systems/WeaponFiringSystem.hpp>
#include <Events/ShootEvent.hpp>
#include <iostream>

namespace hur
{

	WeaponFiringSystem::WeaponFiringSystem(
		hl::EventBus& eventBus,
		hl::Scene& scene
	) :
		_eventBus(eventBus),
		_scene(scene)
	{
		_eventBus.AddListener(this);
	}
	WeaponFiringSystem::~WeaponFiringSystem()
	{
		_eventBus.RemoveListener(this);
	}

	void WeaponFiringSystem::update(float delta)
	{

	}

	void WeaponFiringSystem::OnEvent(const hl::Event& event)
	{
		if (auto se = dynamic_cast<const ShootEvent*>(&event))
		{
			std::cout << "WeaponFiringSystem::ShootEvent" << std::endl;
		}
	}

}