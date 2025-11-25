#pragma once

namespace hl
{
	enum class ServiceLifetime : int
	{
		Transient = 0,
		Scoped = 1,
		Singleton = 2
	};
}