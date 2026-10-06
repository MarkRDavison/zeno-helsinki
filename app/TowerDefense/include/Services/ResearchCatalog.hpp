#pragma once

#include <Services/GraphTypes.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tower
{
	struct ResearchEffect
	{
		std::optional<std::string> tower;
		std::optional<int> startingGoldRank;
		std::optional<int> fireRateRank;
	};

	struct ResearchNode
	{
		std::string id;
		std::string label;
		int cost = 0;
		bool start = false;
		std::optional<GraphPrereq> prereq;
		ResearchEffect effect;
	};

	struct ResearchData
	{
		int version = 1;
		std::vector<ResearchNode> nodes;
	};

	inline std::vector<GraphNode> researchGraphNodes(const ResearchData& data)
	{
		std::vector<GraphNode> nodes;
		nodes.reserve(data.nodes.size());
		for (const auto& node : data.nodes)
		{
			nodes.push_back(GraphNode{ .id = node.id, .prereq = node.prereq });
		}

		return nodes;
	}

	class ResearchCatalog
	{
	public:
		void load(const std::string& path);
		void loadFromText(const std::string& text, const char* file);
		const ResearchData& data() const;
		const ResearchNode* find(std::string_view id) const;
		const std::vector<ResearchNode>& nodes() const;

	private:
		ResearchData _data;
		std::unordered_map<std::string, std::size_t> _byId;
	};
}
