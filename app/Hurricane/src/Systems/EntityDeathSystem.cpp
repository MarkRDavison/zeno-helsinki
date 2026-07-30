#include <Systems/EntityDeathSystem.hpp>
#include <Events/EntityDeathEvent.hpp>

namespace hur
{

	EntityDeathSystem::EntityDeathSystem(
		hl::EventBus& eventBus,
		hl::Scene& scene
	) :
		_eventBus(eventBus),
		_scene(scene)
	{
		_eventBus.AddListener(this);
	}
	EntityDeathSystem::~EntityDeathSystem()
	{
		_eventBus.RemoveListener(this);
	}

	void EntityDeathSystem::update(float delta)
	{

	}

	void EntityDeathSystem::OnEvent(const hl::Event& event)
	{
		if (auto ede = dynamic_cast<const EntityDeathEvent*>(&event))
		{
			auto entity = _scene.getEntity(ede->getId());

			const auto deathType = ede->getDeathType();

			if (entity->HasTag("PLAYER")) // TODO: CONSTANT
			{
				// TODO: WHAT DO WE DO NOW?
				// STATE CHANGE EVENT?
			}

			// TODO: Other on death stuff, drops, decrease lives etc...
			_scene.removeEntity(entity->Id);
		}
	}
}