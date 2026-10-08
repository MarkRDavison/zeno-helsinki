#include <catch2/catch_test_macros.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <algorithm>
#include <vector>

namespace
{
	std::vector<hl::RenderpassInfo> skeletonLikePasses()
	{
		return {
			hl::RenderpassInfo{
				.name = "scene_pass",
				.outputs = {
					hl::ResourceInfo{
						.name = "scene_color",
						.type = hl::ResourceType::Color,
						.format = "VK_FORMAT_B8G8R8A8_SRGB",
						.useMultiSampling = true
					},
					hl::ResourceInfo{
						.name = "scene_depth",
						.type = hl::ResourceType::Depth,
						.format = "VK_FORMAT_D32_SFLOAT",
						.useMultiSampling = true
					},
				},
			},
			hl::RenderpassInfo{
				.name = "ui_pass",
				.outputs = {
					hl::ResourceInfo{
						.name = "ui_color",
						.type = hl::ResourceType::Color,
						.format = "VK_FORMAT_B8G8R8A8_SRGB"
					},
				},
			},
			hl::RenderpassInfo{
				.name = "postprocess_pass",
				.inputs = { "scene_color" },
				.outputs = {
					hl::ResourceInfo{
						.name = "post_color",
						.type = hl::ResourceType::Color,
						.format = "VK_FORMAT_B8G8R8A8_SRGB"
					},
				},
			},
			hl::RenderpassInfo{
				.name = "composite_pass",
				.inputs = { "post_color", "ui_color" },
				.outputs = {
					hl::ResourceInfo{
						.name = "swapchain_color",
						.type = hl::ResourceType::Color,
						.format = "VK_FORMAT_B8G8R8A8_SRGB"
					},
				},
			},
		};
	}

	bool hasEdge(
		const std::vector<hl::GraphImageBarrierEdge>& edges,
		const std::string& passName,
		const std::string& resourceName,
		hl::GraphImageBarrierKind kind)
	{
		return std::any_of(
			edges.begin(),
			edges.end(),
			[&](const hl::GraphImageBarrierEdge& edge)
			{
				return edge.passName == passName
					&& edge.resourceName == resourceName
					&& edge.kind == kind;
			});
	}
}

TEST_CASE("DAG layers: producer color outputs are the next pass inputs", "[graph-barriers]")
{
	const auto passes = skeletonLikePasses();
	const auto nodes = hl::RenderGraph::generateDAG(passes);

	REQUIRE(nodes.at("scene_pass").layer == 0);
	REQUIRE(nodes.at("ui_pass").layer == 0);
	REQUIRE(nodes.at("postprocess_pass").layer == 1);
	REQUIRE(nodes.at("composite_pass").layer == 2);

	REQUIRE(nodes.at("postprocess_pass").prev.size() == 1);
	REQUIRE(nodes.at("postprocess_pass").prev.front() == "scene_pass");
	REQUIRE(std::find(
		nodes.at("composite_pass").prev.begin(),
		nodes.at("composite_pass").prev.end(),
		"postprocess_pass") != nodes.at("composite_pass").prev.end());
}

TEST_CASE("Barrier edges map MSAA scene color to sampled on the post pass", "[graph-barriers]")
{
	const auto edges = hl::RenderGraph::generateImageBarrierEdges(skeletonLikePasses());

	REQUIRE(hasEdge(
		edges,
		"scene_pass",
		"scene_color",
		hl::GraphImageBarrierKind::UndefinedToColorAttachment));
	REQUIRE(hasEdge(
		edges,
		"scene_pass",
		"scene_depth",
		hl::GraphImageBarrierKind::UndefinedToDepthAttachment));
	REQUIRE(hasEdge(
		edges,
		"postprocess_pass",
		"scene_color",
		hl::GraphImageBarrierKind::ColorAttachmentToSampled));
	REQUIRE(hasEdge(
		edges,
		"composite_pass",
		"post_color",
		hl::GraphImageBarrierKind::ColorAttachmentToSampled));
	REQUIRE(hasEdge(
		edges,
		"composite_pass",
		"ui_color",
		hl::GraphImageBarrierKind::ColorAttachmentToSampled));
	REQUIRE(hasEdge(
		edges,
		"composite_pass",
		"swapchain_color",
		hl::GraphImageBarrierKind::ColorAttachmentToPresent));
}
