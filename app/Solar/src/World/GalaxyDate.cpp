#include <World/GalaxyDate.hpp>
#include <algorithm>
#include <chrono>
#include <format>

namespace slr
{

	std::string formatGalaxyDate(long long tick)
	{
		const long long dayIndex = std::max(0ll, tick);
		const std::chrono::year_month_day epoch{ std::chrono::year{ 2200 } / std::chrono::January / 1 };
		const auto date = std::chrono::sys_days{ epoch } + std::chrono::days{ static_cast<int>(dayIndex) };
		const std::chrono::year_month_day ymd{ date };
		return std::format(
			"{:04}-{:02}-{:02}",
			static_cast<int>(ymd.year()),
			static_cast<unsigned>(ymd.month()),
			static_cast<unsigned>(ymd.day()));
	}

}
