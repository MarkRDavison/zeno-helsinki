#pragma once

#include <Services/GraphTypes.hpp>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace tower
{
	using CampaignPrereqKind = GraphPrereqKind;
	using CampaignPrereq = GraphPrereq;

	struct CampaignNode
	{
		std::string id;
		std::string level;
		std::optional<CampaignPrereq> prereq;
		std::optional<std::string> mutex;
	};

	struct CampaignData
	{
		int version = 1;
		std::vector<CampaignNode> nodes;
	};

	struct CampaignProgress
	{
		std::unordered_set<std::string> cleared;
		std::unordered_set<std::string> skipped;
	};

	enum class CampaignNodeState
	{
		Locked,
		Available,
		Cleared,
		Skipped
	};
}
