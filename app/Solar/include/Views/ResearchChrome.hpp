#pragma once

#include <World/GameCommand.hpp>
#include <World/WorldData.hpp>
#include <vector>

namespace slr
{

	class ResearchChrome
	{
	public:
		void setOpen(bool open);
		void toggleOpen();
		void draw(const WorldData& world, std::vector<GameCommand>& pendingCommands);

	private:
		bool _open{ false };
	};

}
