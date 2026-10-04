#pragma once

#include <helsinki/System/Resource/Resource.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>

#include <functional>
#include <string>
#include <vector>

namespace hl
{
	class ResourceManager;

	class LogicalResource : public Resource
	{
	public:
		using ResourceLoader = std::function<bool(const ResourceDefinition::Child&)>;

		LogicalResource(
			const ResourceDefinition& definition,
			ResourceManager& resourceManager,
			ResourceLoader resourceLoader);

		bool Load() override;
		void Unload() override;

		const std::vector<std::string>& GetChildren() const { return _children; }

	private:
		ResourceDefinition _definition;
		ResourceManager& _resourceManager;
		ResourceLoader _resourceLoader;

		std::vector<std::string> _children;
	};
}