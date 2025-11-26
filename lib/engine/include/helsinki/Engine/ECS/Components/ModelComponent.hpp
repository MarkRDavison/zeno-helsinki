#pragma once

#include <string>
#include <helsinki/Engine/ECS/Component.hpp>

namespace hl
{

	class ModelComponent : public Component
	{
	public:
		void setModelId(const std::string& id) { _modelResourceId = id; }
		const std::string& getModelId() const { return _modelResourceId; }

	private:
		std::string _modelResourceId;

	};

}