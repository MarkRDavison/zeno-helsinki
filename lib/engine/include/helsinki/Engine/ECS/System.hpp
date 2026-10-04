#pragma once

#include <helsinki/Engine/ECS/Entity.hpp>

namespace hl
{

	class System
	{
	public:
		virtual ~System() = default;
		virtual void update(float delta) = 0;

	private:

	};

}