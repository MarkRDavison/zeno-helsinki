#pragma once

#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace tower
{
	enum class CampaignPrereqKind
	{
		All,
		Any,
		Node
	};

	struct CampaignPrereq
	{
		CampaignPrereqKind kind = CampaignPrereqKind::All;
		std::string nodeId;
		std::vector<CampaignPrereq> children;
	};

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
