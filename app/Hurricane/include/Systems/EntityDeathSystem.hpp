#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/System/Events/EventBus.hpp>
#include <Services/ResourceService.hpp>
#include <helsinki/System/glm.hpp>
#include <span>

namespace hur
{
	class EntityDeathSystem : public hl::System, public hl::EventListener
	{
	public:
		EntityDeathSystem(
			hl::EventBus& eventBus,
			hl::Scene& scene,
			const ResourceService& resourceService);
		~EntityDeathSystem();
		void update(float delta) override;
		void OnEvent(const hl::Event& event) override;

	private:
		void spawnSpriteClip(
			const glm::vec3& position,
			std::span<const char* const> names,
			float secondsPerFrame);
		void spawnPickup(const glm::vec3& position, const char* pickupId);

	private:
		hl::EventBus& _eventBus;
		hl::Scene& _scene;
		const ResourceService& _resourceService;
	};
}
