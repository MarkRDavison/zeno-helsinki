#pragma once

#include <helsinki/System/Utils/String.hpp>
#include <string>
#include <string_view>

namespace drl
{

	using NeedId = long long;

	inline constexpr float kNeedValueFull = 100.0f;

	inline NeedId needIdFromName(std::string_view name)
	{
		return static_cast<NeedId>(hl::String::fnv1a_32(name));
	}

	struct NeedPrototype
	{
		std::string name;
		float decayPerSecond{ 0.0f };
		float seekBelow{ 0.0f };
		float collapseBelow{ -1.0f };
		int priority{ 0 };
		int order{ 0 };
		std::string label;
	};

	enum class NeedBand
	{
		Ok,
		Seek,
		Collapse
	};

	inline NeedBand classifyNeedValue(float value, const NeedPrototype& prototype)
	{
		if (prototype.collapseBelow >= 0.0f && value <= prototype.collapseBelow)
		{
			return NeedBand::Collapse;
		}
		if (value < prototype.seekBelow)
		{
			return NeedBand::Seek;
		}
		return NeedBand::Ok;
	}

}
