#pragma once

#include <Entities/Need.hpp>
#include <helsinki/System/Utils/String.hpp>
#include <helsinki/System/glm.hpp>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

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

	struct NeedDecayModifier
	{
		float multiplier{ 1.0f };
		float additivePerSecond{ 0.0f };
	};

	struct JobPrototype
	{
		std::string name;
		bool repeats{ false };
		float work{ 0.0f };
		std::function<void(const JobInstance&)> onComplete;
		std::function<glm::vec2(const JobInstance&, const JobPrototype&)> calculateOffset;
		std::unordered_map<NeedId, NeedDecayModifier> needDecay;
	};

}
