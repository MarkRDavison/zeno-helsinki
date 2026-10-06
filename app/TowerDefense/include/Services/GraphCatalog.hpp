#pragma once

#include <Services/GraphTypes.hpp>
#include <helsinki/System/Utils/Json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace tower
{
	GraphPrereq parseGraphPrereqGroup(const hl::JsonNode& obj, const char* file);
	void collectGraphNodeRefs(const GraphPrereq& prereq, std::vector<std::string>& ids);
	void validateGraphPrereqIds(
		const std::vector<GraphNode>& nodes,
		const std::unordered_map<std::string, std::size_t>& byId,
		const char* file);
}
