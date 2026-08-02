#include <Systems/EntityDeathSystem.hpp>
#include <Events/EntityDeathEvent.hpp>
#include <Events/PlayerLifeLostEvent.hpp>
#include <Events/PlayerScoreEvent.hpp>

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

	// TODO: Make systems that dont actually update services???
	void EntityDeathSystem::OnEvent(const hl::Event& event)
	{
		// TODO: Should this just fire entity death events?

		if (auto ede = dynamic_cast<const EntityDeathEvent*>(&event))
		{
			auto entity = _scene.getEntity(ede->getId());

			const auto deathType = ede->getDeathType();

			if (entity->HasTag("PLAYER")) // TODO: CONSTANT
			{
				_eventBus.PublishEvent(PlayerLifeLostEvent());
			}
			else
			{
				// TODO: Different for enemy and world/environment???
				_eventBus.PublishEvent(PlayerScoreEvent(1));
			}

			// TODO: Other on death stuff, drops, decrease lives etc...
			_scene.removeEntity(entity->Id);
		}
	}
}