#pragma once

#include <Team.hpp>
#include <helsinki/Engine/ECS/Component.hpp>
#include <helsinki/Engine/ECS/Entity.hpp>

namespace tower
{
	class TeamComponent : public hl::Component
	{
	public:
		Team team = Team::Neutral;
	};

	inline bool hasTeam(const hl::Entity* entity, Team team)
	{
		if (entity == nullptr)
		{
			return false;
		}

		const auto* component = entity->GetComponent<TeamComponent>();
		return component != nullptr && component->team == team;
	}

	inline bool hasTeam(const hl::Entity& entity, Team team)
	{
		return hasTeam(&entity, team);
	}
}
