#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/System/Events/EventBus.hpp>
#include <helsinki/System/glm.hpp>
#include <Services/ResourceService.hpp>
#include <span>

namespace hur
{
	class BombSystem : public hl::System, public hl::EventListener
	{
	public:
		BombSystem(
			hl::EventBus& eventBus,
			hl::Scene& scene,
			const ResourceService& resourceService);
		~BombSystem();
		void update(float delta) override;
		void OnEvent(const hl::Event& event) override;

	private:
		void spawnSpriteClip(
			const glm::vec3& position,
			std::span<const char* const> names,
			float secondsPerFrame);
		void spawnMissile(const glm::vec3& playerPosition);
		void detonateAt(const glm::vec3& position);

		hl::EventBus& _eventBus;
		hl::Scene& _scene;
		const ResourceService& _resourceService;
	};
}
