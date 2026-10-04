#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/Engine/Input/InputManager.hpp>
#include <helsinki/System/Events/EventBus.hpp>
#include <Services/ResourceService.hpp>

namespace hur
{
	class PlayerControlSystem : public hl::System
	{
	public:
		PlayerControlSystem(
			hl::InputManager& inputManager,
			hl::EventBus& eventBus,
			hl::Scene& scene,
			const ResourceService& resourceService);
		void update(float delta) override;

	private:
		void spawnPlayerEngine();
		void syncPlayerEngine(const hl::Entity& player);
		void destroyPlayerEngine();
		void spawnPlayerShield();
		void syncPlayerShield(const hl::Entity& player, int shieldLayers);
		void destroyPlayerShield();

		hl::InputManager& _inputManager;
		hl::EventBus& _eventBus;
		hl::Scene& _scene;
		const ResourceService& _resourceService;
	};

}
