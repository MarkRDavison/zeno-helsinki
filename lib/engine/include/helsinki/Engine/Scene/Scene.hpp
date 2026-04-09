#pragma once

#include <helsinki/Engine/ECS/Entity.hpp>
#include <helsinki/Engine/ECS/System.hpp>
#include <algorithm>
#include <iterator>

namespace hl
{

	class Scene
	{
	private:

		template <class InputIt, class OutputIt, class Pred, class Fct>
		static inline void transform_if(InputIt first, InputIt last, OutputIt dest, Pred pred, Fct transform)
		{
			while (first != last)
			{
				if (pred(*first))
				{
					*dest++ = transform(*first);
				}

				++first;
			}
		}

	public:
		Entity* addEntity(const std::string& name);

		Entity* getEntity(const std::string& name);

		void removeEntity(const std::string& name);

		void addSystem(System* system);

		const std::vector<std::unique_ptr<Entity>>& getEntities() const;
		std::vector<std::unique_ptr<Entity>>& getEntities();

		std::vector<Entity*> getEntitiesByTag(const std::string& tag) const;

		template<typename... Args>
		std::vector<Entity*> getEntitiesWithComponents()const
		{
			std::vector<Entity*> entities;

			transform_if(
				std::cbegin(_entities),
				std::cend(_entities),
				std::back_inserter(entities),
				[&](const std::unique_ptr<Entity>& o) { return o->HasComponents<Args...>(); },
				[](const std::unique_ptr<Entity>& o) { return o.get(); });

			return entities;
		}
		template<typename... Args>
		std::vector<Entity*> getEntitiesWithComponents(const std::string& tag)const
		{
			std::vector<Entity*> entities;

			transform_if(
				std::cbegin(_entities),
				std::cend(_entities),
				std::back_inserter(entities),
				[&](const std::unique_ptr<Entity>& o) { return o->HasTag(tag) && o->HasComponents<Args...>(); },
				[](const std::unique_ptr<Entity>& o) { return o.get(); });

			return entities;
		}

		void update(float delta);

	private:
		std::vector<std::unique_ptr<Entity>> _entities;
		std::vector<std::unique_ptr<System>> _systems;
	};

}