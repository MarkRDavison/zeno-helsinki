#include <catch2/catch_test_macros.hpp>
#include <Services/CampaignCatalog.hpp>
#include <Services/CampaignEvaluator.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using tower::CampaignCatalog;
using tower::CampaignData;
using tower::CampaignNode;
using tower::CampaignNodeState;
using tower::CampaignPrereq;
using tower::CampaignPrereqKind;
using tower::CampaignProgress;
using tower::evaluateCampaign;

namespace
{
	CampaignPrereq nodeRef(std::string id)
	{
		CampaignPrereq prereq;
		prereq.kind = CampaignPrereqKind::Node;
		prereq.nodeId = std::move(id);
		return prereq;
	}

	CampaignPrereq allOf(std::vector<CampaignPrereq> children)
	{
		CampaignPrereq prereq;
		prereq.kind = CampaignPrereqKind::All;
		prereq.children = std::move(children);
		return prereq;
	}

	CampaignPrereq anyOf(std::vector<CampaignPrereq> children)
	{
		CampaignPrereq prereq;
		prereq.kind = CampaignPrereqKind::Any;
		prereq.children = std::move(children);
		return prereq;
	}

	CampaignNode makeNode(
		std::string id,
		std::optional<CampaignPrereq> prereq = std::nullopt,
		std::optional<std::string> mutex = std::nullopt)
	{
		CampaignNode node;
		node.id = std::move(id);
		node.level = "levels/fake.json";
		node.prereq = std::move(prereq);
		node.mutex = std::move(mutex);
		return node;
	}

	CampaignData chain()
	{
		CampaignData data;
		data.nodes.push_back(makeNode("n1"));
		data.nodes.push_back(makeNode("n2", allOf({ nodeRef("n1") })));
		data.nodes.push_back(makeNode("n3", allOf({ nodeRef("n2") })));
		return data;
	}

	CampaignNodeState stateOf(
		const std::unordered_map<std::string, CampaignNodeState>& states,
		const char* id)
	{
		const auto it = states.find(id);
		REQUIRE(it != states.end());
		return it->second;
	}
}

TEST_CASE("chain empty progress: first available, rest locked", "[tower][campaign]")
{
	const auto states = evaluateCampaign(chain(), {});
	CHECK(stateOf(states, "n1") == CampaignNodeState::Available);
	CHECK(stateOf(states, "n2") == CampaignNodeState::Locked);
	CHECK(stateOf(states, "n3") == CampaignNodeState::Locked);
}

TEST_CASE("chain first cleared: second available", "[tower][campaign]")
{
	CampaignProgress progress;
	progress.cleared = { "n1" };
	const auto states = evaluateCampaign(chain(), progress);
	CHECK(stateOf(states, "n1") == CampaignNodeState::Cleared);
	CHECK(stateOf(states, "n2") == CampaignNodeState::Available);
	CHECK(stateOf(states, "n3") == CampaignNodeState::Locked);
}

TEST_CASE("chain two cleared: third available", "[tower][campaign]")
{
	CampaignProgress progress;
	progress.cleared = { "n1", "n2" };
	const auto states = evaluateCampaign(chain(), progress);
	CHECK(stateOf(states, "n1") == CampaignNodeState::Cleared);
	CHECK(stateOf(states, "n2") == CampaignNodeState::Cleared);
	CHECK(stateOf(states, "n3") == CampaignNodeState::Available);
}

TEST_CASE("any rejoin after one fork is cleared", "[tower][campaign]")
{
	CampaignData data;
	data.nodes.push_back(makeNode("root"));
	data.nodes.push_back(makeNode("A", allOf({ nodeRef("root") })));
	data.nodes.push_back(makeNode("B", allOf({ nodeRef("root") })));
	data.nodes.push_back(makeNode("C", anyOf({ nodeRef("A"), nodeRef("B") })));

	CampaignProgress progress;
	progress.cleared = { "root", "A" };
	const auto states = evaluateCampaign(data, progress);
	CHECK(stateOf(states, "A") == CampaignNodeState::Cleared);
	CHECK(stateOf(states, "B") == CampaignNodeState::Available);
	CHECK(stateOf(states, "C") == CampaignNodeState::Available);
}

TEST_CASE("mutex skip then any rejoin", "[tower][campaign]")
{
	CampaignData data;
	data.nodes.push_back(makeNode("root"));
	data.nodes.push_back(makeNode("A", allOf({ nodeRef("root") }), "pick"));
	data.nodes.push_back(makeNode("B", allOf({ nodeRef("root") }), "pick"));
	data.nodes.push_back(makeNode("C", anyOf({ nodeRef("A"), nodeRef("B") })));

	CampaignProgress progress;
	progress.cleared = { "root", "A" };
	const auto states = evaluateCampaign(data, progress);
	CHECK(stateOf(states, "A") == CampaignNodeState::Cleared);
	CHECK(stateOf(states, "B") == CampaignNodeState::Skipped);
	CHECK(stateOf(states, "C") == CampaignNodeState::Available);
}

TEST_CASE("mutex plus all successor is skipped", "[tower][campaign]")
{
	CampaignData data;
	data.nodes.push_back(makeNode("root"));
	data.nodes.push_back(makeNode("A", allOf({ nodeRef("root") }), "pick"));
	data.nodes.push_back(makeNode("B", allOf({ nodeRef("root") }), "pick"));
	data.nodes.push_back(makeNode("C", allOf({ nodeRef("A"), nodeRef("B") })));

	CampaignProgress progress;
	progress.cleared = { "root", "A" };
	const auto states = evaluateCampaign(data, progress);
	CHECK(stateOf(states, "B") == CampaignNodeState::Skipped);
	CHECK(stateOf(states, "C") == CampaignNodeState::Skipped);
}

TEST_CASE("nested any of all-or-D", "[tower][campaign]")
{
	CampaignData data;
	data.nodes.push_back(makeNode("A"));
	data.nodes.push_back(makeNode("B"));
	data.nodes.push_back(makeNode("D"));
	data.nodes.push_back(makeNode(
		"C",
		anyOf({ allOf({ nodeRef("A"), nodeRef("B") }), nodeRef("D") })));

	SECTION("A and B cleared")
	{
		CampaignProgress progress;
		progress.cleared = { "A", "B" };
		const auto states = evaluateCampaign(data, progress);
		CHECK(stateOf(states, "C") == CampaignNodeState::Available);
	}

	SECTION("only D cleared")
	{
		CampaignProgress progress;
		progress.cleared = { "D" };
		const auto states = evaluateCampaign(data, progress);
		CHECK(stateOf(states, "C") == CampaignNodeState::Available);
	}

	SECTION("only A cleared")
	{
		CampaignProgress progress;
		progress.cleared = { "A" };
		const auto states = evaluateCampaign(data, progress);
		CHECK(stateOf(states, "C") == CampaignNodeState::Locked);
		CHECK(stateOf(states, "B") == CampaignNodeState::Available);
		CHECK(stateOf(states, "D") == CampaignNodeState::Available);
	}

	SECTION("D skipped and A incomplete")
	{
		CampaignProgress progress;
		progress.skipped = { "D" };
		progress.cleared = { "A" };
		const auto states = evaluateCampaign(data, progress);
		CHECK(stateOf(states, "D") == CampaignNodeState::Skipped);
		CHECK(stateOf(states, "C") == CampaignNodeState::Locked);
	}

	SECTION("D skipped and A+B still open is locked; both skipped then C skipped")
	{
		CampaignProgress progress;
		progress.skipped = { "D", "B" };
		const auto states = evaluateCampaign(data, progress);
		CHECK(stateOf(states, "C") == CampaignNodeState::Skipped);
	}
}

TEST_CASE("campaign.json chain parses and evaluates", "[tower][campaign][catalog]")
{
	constexpr auto text = R"json(
{
  "version": 1,
  "nodes": [
    { "id": "level-1", "level": "levels/level-1.json" },
    { "id": "level-2", "level": "levels/level-2.json", "prereq": { "all": ["level-1"] } },
    { "id": "level-3", "level": "levels/level-3.json", "prereq": { "all": ["level-2"] } }
  ]
}
)json";

	CampaignCatalog catalog;
	catalog.loadFromText(text, "campaign.json");
	REQUIRE(catalog.nodes().size() == 3);
	REQUIRE(catalog.find("level-1") != nullptr);
	CHECK_FALSE(catalog.find("level-1")->prereq.has_value());
	CHECK(catalog.find("level-2")->prereq.has_value());

	const auto states = evaluateCampaign(catalog.data(), {});
	CHECK(stateOf(states, "level-1") == CampaignNodeState::Available);
	CHECK(stateOf(states, "level-2") == CampaignNodeState::Locked);
	CHECK(stateOf(states, "level-3") == CampaignNodeState::Locked);
}

TEST_CASE("catalog parses nested any", "[tower][campaign][catalog]")
{
	constexpr auto text = R"json(
{
  "version": 1,
  "nodes": [
    { "id": "A", "level": "a.json" },
    { "id": "B", "level": "b.json" },
    { "id": "D", "level": "d.json" },
    { "id": "C", "level": "c.json", "prereq": { "any": [ { "all": ["A", "B"] }, "D" ] } }
  ]
}
)json";

	CampaignCatalog catalog;
	catalog.loadFromText(text, "campaign.json");
	const auto* c = catalog.find("C");
	REQUIRE(c != nullptr);
	REQUIRE(c->prereq.has_value());
	CHECK(c->prereq->kind == CampaignPrereqKind::Any);
	REQUIRE(c->prereq->children.size() == 2);
	CHECK(c->prereq->children[0].kind == CampaignPrereqKind::All);
	CHECK(c->prereq->children[1].kind == CampaignPrereqKind::Node);
}

TEST_CASE("catalog rejects unknown prereq id", "[tower][campaign][catalog]")
{
	constexpr auto text = R"json(
{
  "version": 1,
  "nodes": [
    { "id": "A", "level": "a.json", "prereq": { "all": ["missing"] } }
  ]
}
)json";

	CampaignCatalog catalog;
	CHECK_THROWS_AS(catalog.loadFromText(text, "campaign.json"), std::runtime_error);
}
