#pragma once

#include <World/GameCommand.hpp>
#include <World/WorldData.hpp>
#include <vector>

namespace slr
{

	class EngagementPolicyChrome
	{
	public:
		void setOpen(bool open);
		void draw(const WorldData& world, std::vector<GameCommand>& pendingCommands);

	private:
		bool _open{ false };
		long long _draftId{ 0 };
		char _draftName[64]{};
		int _draftRuleIndex{ 0 };
	};

}
