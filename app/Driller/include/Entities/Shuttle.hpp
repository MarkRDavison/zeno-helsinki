#pragma once

#include <helsinki/System/glm.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace drl
{

	using ShuttleId = long long;
	using ShuttlePrototypeId = long long;

	inline constexpr glm::vec2 kShuttleStartingPosition{ -50.0f, -20.0f };
	inline constexpr glm::vec2 kShuttleSurfacePosition{ -4.0f, 0.0f };
	inline constexpr glm::vec2 kShuttleLeavingPosition{ 50.0f, -20.0f };

	enum class ShuttleState
	{
		Idle,
		TravellingToSurface,
		WaitingOnSurface,
		LeavingSurface,
		Completed
	};

	struct ShuttleInstance
	{
		ShuttleId id{ 0 };
		ShuttlePrototypeId prototypeId{ 0 };
		ShuttleState state{ ShuttleState::Idle };
		float elapsed{ 0.0f };
		glm::vec2 position{ 0.0f, 0.0f };
		glm::vec2 startingPosition{ 0.0f, 0.0f };
		glm::vec2 surfacePosition{ 0.0f, 0.0f };
		glm::vec2 leavingPosition{ 0.0f, 0.0f };
		std::unordered_map<std::string, long long> cargo;
	};

	struct ShuttlePrototype
	{
		std::string name;
		glm::ivec2 size{ 0, 0 };
		glm::ivec2 texture{ 0, 0 };
		float idleTime{ 0.0f };
		float loadingTime{ 0.0f };
		float speed{ 0.0f };
		std::unordered_set<std::string> allowedCargo;
	};

}
