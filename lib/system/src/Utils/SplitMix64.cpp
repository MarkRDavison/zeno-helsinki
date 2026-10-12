#include <helsinki/System/Utils/SplitMix64.hpp>

namespace hl
{

	std::uint64_t splitMix64(std::uint64_t& state)
	{
		std::uint64_t z = (state += 0x9E3779B97F4A7C15ull);
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}

	float splitMix64Unit(std::uint64_t& state)
	{
		return static_cast<float>((splitMix64(state) >> 11) * 0x1.0p-53);
	}

	std::uint64_t splitMix64Seed(std::span<const std::uint64_t> parts)
	{
		std::uint64_t state = 0;
		for (const std::uint64_t part : parts)
		{
			state ^= part;
			splitMix64(state);
		}

		return state;
	}

}
