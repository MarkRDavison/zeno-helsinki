#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/System/Events/EventBus.hpp>
#include <Services/ResourceService.hpp>

namespace hur
{

	class EnemySpawnSystem : public hl::System, public hl::EventListener
	{
	public:
		EnemySpawnSystem(
			hl::EventBus& eventBus,
			hl::Scene& scene,
			const ResourceService& resourceService);
		~EnemySpawnSystem();
		void update(float delta) override;
		void OnEvent(const hl::Event& event) override;

	private:
		void spawnDefaultEnemy();

	private:
		hl::EventBus& _eventBus;
		hl::Scene& _scene;
		const ResourceService& _resourceService;
		float _elapsed{ 0.0f };
	};

}