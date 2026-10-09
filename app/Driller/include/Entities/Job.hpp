#pragma once

#include <helsinki/System/Utils/String.hpp>
#include <helsinki/System/glm.hpp>
#include <functional>
#include <sol/sol.hpp>
#include <string>
#include <string_view>

namespace drl
{

	using JobId = long long;
	using JobPrototypeId = long long;

	inline JobPrototypeId jobPrototypeIdFromName(std::string_view name)
	{
		return static_cast<JobPrototypeId>(hl::String::fnv1a_32(name));
	}

	struct JobInstance
	{
		JobId id{ 0 };
		JobPrototypeId prototypeId{ 0 };
		JobPrototypeId additionalPrototypeId{ 0 };
		long long allocatedWorkerId{ 0 };
		glm::ivec2 tile{ 0, 0 };
		glm::vec2 offset{ 0.0f, 0.0f };
		bool requiresRemoval{ false };
		float work{ 0.0f };
	};

	struct JobPrototype
	{
		std::string name;
		bool repeats{ false };
		float work{ 0.0f };
		sol::protected_function onComplete;
		std::function<glm::vec2(const JobInstance&, const JobPrototype&)> calculateOffset;
	};

}
