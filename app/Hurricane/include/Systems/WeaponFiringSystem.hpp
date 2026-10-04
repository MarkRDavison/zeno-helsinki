#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/System/Events/EventBus.hpp>
#include <helsinki/System/glm.hpp>
#include <Services/ResourceService.hpp>
#include <WeaponCatalog.hpp>

namespace hur
{

	class WeaponFiringSystem : public hl::System, public hl::EventListener
	{
	public:
		WeaponFiringSystem(
			hl::EventBus& eventBus,
			hl::Scene& scene,
			const ResourceService& resourceService);
		~WeaponFiringSystem();
		void update(float delta) override;
		void OnEvent(const hl::Event& event) override;
	private:
		void spawnProjectile(
			const glm::vec3& position,
			const WeaponComponent& weapon,
			const hl::Entity& shooter);

		hl::EventBus& _eventBus;
		hl::Scene& _scene;
		const ResourceService& _resourceService;
	};

}