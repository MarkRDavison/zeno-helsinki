#pragma once

#include <Services/CampaignTypes.hpp>
#include <string>
#include <unordered_map>

namespace tower
{
	std::unordered_map<std::string, CampaignNodeState> evaluateCampaign(
		const CampaignData& data,
		const CampaignProgress& progress);
}
