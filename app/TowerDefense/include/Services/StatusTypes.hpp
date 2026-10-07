#pragma once

#include <helsinki/System/glm.hpp>
#include <string>

namespace tower
{
	enum class StatusKind
	{
		Dot,
		Stat
	};

	struct StatusCategoryDef
	{
		std::string id;
		StatusKind kind = StatusKind::Stat;
		glm::vec3 color{ 1.0f };
		int cap = 1;
	};

	struct StatusDef
	{
		std::string id;
		std::string category;
		StatusKind kind = StatusKind::Stat;
		float duration = 0.0f;
		int score = 0;
		int cap = 1;
		float interval = 0.0f;
		float tickDamage = 0.0f;
		std::string damageType;
		float magnitude = 0.0f;
		std::string channel;
	};

	inline bool isUnitChannel(std::string_view channel)
	{
		return channel == "speed" || channel == "health" || channel == "weakness";
	}
}
