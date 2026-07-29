#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/System/Events/EventBus.hpp>

namespace hur
{

	class WeaponFiringSystem : public hl::System, public hl::EventListener
	{
	public:
		WeaponFiringSystem(
			hl::EventBus& eventBus,
			hl::Scene& scene);
		~WeaponFiringSystem();
		void update(float delta) override;
		void OnEvent(const hl::Event& event) override;
	private:
		hl::EventBus& _eventBus;
		hl::Scene& _scene;
	};

}