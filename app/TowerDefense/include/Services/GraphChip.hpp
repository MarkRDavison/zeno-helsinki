#pragma once

#include <Services/CampaignTypes.hpp>
#include <Services/GraphTypes.hpp>
#include <helsinki/System/glm.hpp>

namespace tower
{
	inline glm::vec3 graphChipColor(GraphNodeState state)
	{
		switch (state)
		{
		case GraphNodeState::Available:
			return { 1.0f, 0.5f, 0.0f };
		case GraphNodeState::Completed:
			return { 0.2f, 0.75f, 0.55f };
		case GraphNodeState::Skipped:
			return { 0.16f, 0.16f, 0.18f };
		case GraphNodeState::Locked:
		default:
			return { 0.28f, 0.28f, 0.32f };
		}
	}

	inline GraphNodeState campaignStateAsGraph(CampaignNodeState state)
	{
		switch (state)
		{
		case CampaignNodeState::Available:
			return GraphNodeState::Available;
		case CampaignNodeState::Cleared:
			return GraphNodeState::Completed;
		case CampaignNodeState::Skipped:
			return GraphNodeState::Skipped;
		case CampaignNodeState::Locked:
		default:
			return GraphNodeState::Locked;
		}
	}
}
