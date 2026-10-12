#pragma once

#include <helsinki/System/Export.hpp>
#include <cstdint>
#include <initializer_list>
#include <span>

namespace hl
{

	HELSINKI_SYSTEM_EXPORT std::uint64_t splitMix64(std::uint64_t& state);
	HELSINKI_SYSTEM_EXPORT float splitMix64Unit(std::uint64_t& state);
	HELSINKI_SYSTEM_EXPORT std::uint64_t splitMix64Seed(std::span<const std::uint64_t> parts);

	inline std::uint64_t splitMix64Seed(std::initializer_list<std::uint64_t> parts)
	{
		return splitMix64Seed(std::span<const std::uint64_t>(parts.begin(), parts.size()));
	}

}
