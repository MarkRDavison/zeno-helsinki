#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/System/Events/EventBus.hpp>
#include <Events/EntityDeathEvent.hpp>

namespace hur
{

	class CollisionResolutionSystem : public hl::System, public hl::EventListener
	{
	public:
		CollisionResolutionSystem(
			hl::EventBus& eventBus,
			hl::Scene& scene);
		~CollisionResolutionSystem();
		void update(float delta) override;
		void OnEvent(const hl::Event& event) override;

	private:
		void applyDamageToEntity(hl::Entity* entity, int damage, DeathType type);

	private:
		hl::EventBus& _eventBus;
		hl::Scene& _scene;
	};

}