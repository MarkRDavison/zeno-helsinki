#pragma once

#include <string>
#include <unordered_set>
#include <vector>

namespace tower
{
	struct MatchContext
	{
		bool campaign = false;
		std::string nodeId;
		std::vector<std::string> ownedTowers;
	};

	inline std::vector<std::string> matchPlaceableTowerIds(
		const std::vector<std::string>& catalogOrder,
		const MatchContext& match)
	{
		if (!match.campaign)
		{
			return catalogOrder;
		}

		const std::unordered_set<std::string> owned(
			match.ownedTowers.begin(),
			match.ownedTowers.end());
		std::vector<std::string> ids;
		ids.reserve(catalogOrder.size());
		for (const auto& id : catalogOrder)
		{
			if (owned.contains(id))
			{
				ids.push_back(id);
			}
		}
		return ids;
	}
}
