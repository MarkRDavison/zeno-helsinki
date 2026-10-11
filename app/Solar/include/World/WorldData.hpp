#pragma once

#include <helsinki/System/glm.hpp>
#include <string>
#include <vector>

namespace slr
{

	struct StarSystem
	{
		long long id{ 0 };
		std::string name;
		glm::vec2 position{ 0.0f };
	};

	struct Hyperlane
	{
		long long a{ 0 };
		long long b{ 0 };
	};

	struct WorldData
	{
		long long tick{ 0 };
		std::vector<StarSystem> systems;
		std::vector<Hyperlane> hyperlanes;

		const StarSystem* systemById(long long id) const;
	};

}
