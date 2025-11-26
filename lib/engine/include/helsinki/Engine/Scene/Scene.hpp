#pragma once

#include <helsinki/Engine/ECS/Entity.hpp>
#include <helsinki/Engine/ECS/System.hpp>

namespace hl
{

	class Scene
	{
	public:
		Entity* addEntity(const std::string& name);

		Entity* getEntity(const std::string& name);

		void removeEntity(const std::string& name);

		void addSystem(System* system);

		const std::vector<std::unique_ptr<Entity>>& getEntities() const;
		std::vector<std::unique_ptr<Entity>>& getEntities();

	private:
		std::vector<std::unique_ptr<Entity>> _entities;
		std::vector<std::unique_ptr<System>> _systems;
	};

}