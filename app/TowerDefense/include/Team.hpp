#pragma once

namespace tower
{
	enum class Team
	{
		Tower,
		Creep,
		Neutral
	};

	inline bool towerAutoTargets(Team team)
	{
		return team == Team::Creep;
	}
}
