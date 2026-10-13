#pragma once

#include <World/GameCommand.hpp>
#include <World/ShipDesign.hpp>
#include <World/WorldData.hpp>
#include <vector>

namespace slr
{

	class DesignChrome
	{
	public:
		void setOpen(bool open);
		void toggleOpen();
		void draw(const WorldData& world, std::vector<GameCommand>& pendingCommands);

	private:
		void resetDraftToNewClass();

		bool _open{ false };
		char _draftName[64]{};
		int _hullSizeIndex{ 0 };
		ShipDesignPartCounts _parts;
	};

}
