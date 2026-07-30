#include <helsinki/Engine/Scene/Scene.hpp>

namespace hl
{
	Entity* Scene::addEntity()
	{
		_entities.emplace_back(std::make_unique<Entity>(_nextId++));

		auto& ePtr = _entities.back();

		return ePtr.get();
	}

	Entity* Scene::addEntity(const std::string& name)
	{
		if (auto e = getEntity(name); e != nullptr)
		{
			return e;
		}

		auto e = addEntity();
		e->setName(name);
		return e;
	}

	Entity* Scene::getEntity(const std::string& name)
	{
		auto it = std::find_if(
			_entities.begin(),
			_entities.end(),
			[&name](const std::unique_ptr<Entity>& e)
			{
				return name == e->getName();
			});

		if (it == _entities.end())
		{
			return nullptr;
		}

		return (*it).get();
	}

	Entity* Scene::getEntity(int id)
	{
		auto it = std::find_if(
			_entities.begin(),
			_entities.end(),
			[&id](const std::unique_ptr<Entity>& e)
			{
				return id == e->Id;
			});

		if (it == _entities.end())
		{
			return nullptr;
		}

		return (*it).get();
	}

	void Scene::removeEntity(const std::string& name)
	{
		if (auto e = getEntity(name); e != nullptr)
		{
			_entitiesToRemove.insert(e->Id);
		}
	}

	void Scene::removeEntity(int id)
	{
		if (auto e = getEntity(id); e != nullptr)
		{
			_entitiesToRemove.insert(id);
		}
	}

	void Scene::update()
	{
		if (!_entitiesToRemove.empty())
		{
			std::erase_if(_entities, [&](const std::unique_ptr<Entity>& e)
				{
					return _entitiesToRemove.contains(e->Id);
				});

			_entitiesToRemove.clear();
		}
	}

	void Scene::addSystem(System* system)
	{
		_systems.emplace_back(std::move(system));
	}

	const std::vector<std::unique_ptr<Entity>>& Scene::getEntities() const
	{
		return _entities;
	}
	std::vector<std::unique_ptr<Entity>>& Scene::getEntities()
	{
		return _entities;
	}

	std::vector<Entity*> Scene::getEntitiesByTag(const std::string& tag) const
	{
		std::vector<Entity*> taggedEntities;

		transform_if(
			std::cbegin(_entities),
			std::cend(_entities),
			std::back_inserter(taggedEntities),
			[&](const std::unique_ptr<Entity>& o) { return o->HasTag(tag); },
			[](const std::unique_ptr<Entity>& o) { return o.get(); });

		return taggedEntities;
	}

	void Scene::update(float delta)
	{
		for (auto& system : _systems)
		{
			system->update(delta);
		}
	}
}