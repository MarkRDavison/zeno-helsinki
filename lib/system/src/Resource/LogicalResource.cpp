#include <helsinki/System/Resource/LogicalResource.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>

namespace hl
{
	LogicalResource::LogicalResource(
		const ResourceDefinition& definition,
		ResourceManager& resourceManager,
		ResourceLoader resourceLoader)
		:
		Resource(definition.name),
		_definition(definition),
		_resourceManager(resourceManager),
		_resourceLoader(std::move(resourceLoader))
	{
	}

	bool LogicalResource::Load()
	{
		for (const auto& child : _definition.resources)
		{
			if (!_resourceLoader(child))
			{
				Unload();
				return false;
			}

			_children.push_back(child.name);
		}

		return Resource::Load();
	}

	void LogicalResource::Unload()
	{
		for (const auto& child : _children)
		{
			_resourceManager.Release(child);
		}

		_children.clear();

		Resource::Unload();
	}
}