#pragma once

#include <string>
#include <PongConstants.hpp>
#include <helsinki/Engine/ECS/Component.hpp>
#include <helsinki/System/glm.hpp>

namespace pong
{

	class EntityComponent : public hl::Component
	{
	public:
		std::string VertexBufferResourceName;
		glm::vec4 Color = glm::vec4(1.0, 0.0, 0.0, 1.0);
		ControlledState ControlledState = ControlledState::NONE;
	private:

	};

}