#include <catch2/catch_test_macros.hpp>
#include <Services/GraphEvaluator.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using tower::GraphNode;
using tower::GraphNodeState;
using tower::GraphPrereq;
using tower::GraphPrereqKind;
using tower::evaluateGraph;

namespace
{
	GraphPrereq nodeRef(std::string id)
	{
		GraphPrereq prereq;
		prereq.kind = GraphPrereqKind::Node;
		prereq.nodeId = std::move(id);
		return prereq;
	}

	GraphPrereq allOf(std::vector<GraphPrereq> children)
	{
		GraphPrereq prereq;
		prereq.kind = GraphPrereqKind::All;
		prereq.children = std::move(children);
		return prereq;
	}

	GraphPrereq anyOf(std::vector<GraphPrereq> children)
	{
		GraphPrereq prereq;
		prereq.kind = GraphPrereqKind::Any;
		prereq.children = std::move(children);
		return prereq;
	}

	GraphNode makeNode(std::string id, std::optional<GraphPrereq> prereq = std::nullopt)
	{
		GraphNode node;
		node.id = std::move(id);
		node.prereq = std::move(prereq);
		return node;
	}

	GraphNodeState stateOf(
		const std::unordered_map<std::string, GraphNodeState>& states,
		const std::string& id)
	{
		const auto it = states.find(id);
		REQUIRE(it != states.end());
		return it->second;
	}
}

TEST_CASE("evaluateGraph any: one of two completed unlocks rejoin", "[tower][graph]")
{
	std::vector<GraphNode> nodes;
	nodes.push_back(makeNode("A"));
	nodes.push_back(makeNode("B"));
	nodes.push_back(makeNode("C", anyOf({ nodeRef("A"), nodeRef("B") })));

	const auto locked = evaluateGraph(nodes, {});
	CHECK(stateOf(locked, "A") == GraphNodeState::Available);
	CHECK(stateOf(locked, "C") == GraphNodeState::Locked);

	const auto open = evaluateGraph(nodes, { "A" });
	CHECK(stateOf(open, "A") == GraphNodeState::Completed);
	CHECK(stateOf(open, "C") == GraphNodeState::Available);
}

TEST_CASE("evaluateGraph all: both required", "[tower][graph]")
{
	std::vector<GraphNode> nodes;
	nodes.push_back(makeNode("A"));
	nodes.push_back(makeNode("B"));
	nodes.push_back(makeNode("C", allOf({ nodeRef("A"), nodeRef("B") })));

	const auto one = evaluateGraph(nodes, { "A" });
	CHECK(stateOf(one, "C") == GraphNodeState::Locked);

	const auto both = evaluateGraph(nodes, { "A", "B" });
	CHECK(stateOf(both, "C") == GraphNodeState::Available);
}
