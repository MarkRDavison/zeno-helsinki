#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace tower
{
	struct ClampResistResult
	{
		float value = 0.0f;
		bool warned = false;
	};

	inline ClampResistResult clampResist(float resist)
	{
		if (resist > 1.0f)
		{
			return { 1.0f, true };
		}

		return { resist, false };
	}

	inline float resistOf(
		const std::unordered_map<std::string, float>& resist,
		std::string_view type)
	{
		const auto it = resist.find(std::string(type));
		if (it == resist.end())
		{
			return 0.0f;
		}

		return it->second;
	}

	inline float takenDamage(float damage, float resist)
	{
		const float taken = damage * (1.0f - clampResist(resist).value);
		if (taken < 0.0f)
		{
			return 0.0f;
		}

		return taken;
	}

	inline bool isDead(float current)
	{
		return current <= 0.0f;
	}

	inline void applyHit(float& current, float damage, float resist)
	{
		current -= takenDamage(damage, resist);
	}
}
