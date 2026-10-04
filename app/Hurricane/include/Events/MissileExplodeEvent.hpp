#pragma once

#include <helsinki/System/Events/Event.hpp>
#include <helsinki/System/glm.hpp>

namespace hur
{
	class MissileExplodeEvent : public hl::Event
	{
	public:
		MissileExplodeEvent(glm::vec3 position) : _position(position) {}

		glm::vec3 getPosition() const { return _position; }

		DEFINE_EVENT_TYPE(MissileExplodeEvent)

	private:
		glm::vec3 _position;
	};

}
