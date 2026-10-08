#include <catch2/catch_test_macros.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>

TEST_CASE("passIsCompute is false for graphics passes", "[compute]")
{
	const hl::RenderpassInfo pass{
		.name = "scene",
		.outputs = {
			hl::ResourceInfo{ .name = "color", .type = hl::ResourceType::Color }
		},
		.pipelineGroups = {
			{
				hl::PipelineInfo{
					.name = "model_pipeline",
					.shaderVert = "a.vert",
					.shaderFrag = "a.frag",
					.bindPoint = hl::PipelineBindPoint::Graphics
				}
			}
		}
	};

	REQUIRE_FALSE(hl::passIsCompute(pass));
}

TEST_CASE("passIsCompute is true for compute with no color outputs", "[compute]")
{
	const hl::RenderpassInfo pass{
		.name = "noop",
		.pipelineGroups = {
			{
				hl::PipelineInfo{
					.name = "noop_compute_pipeline",
					.shaderComp = "noop.comp",
					.bindPoint = hl::PipelineBindPoint::Compute
				}
			}
		}
	};

	REQUIRE(hl::passIsCompute(pass));
}

TEST_CASE("passIsCompute throws when compute has color outputs", "[compute]")
{
	const hl::RenderpassInfo pass{
		.name = "bad",
		.outputs = {
			hl::ResourceInfo{ .name = "color", .type = hl::ResourceType::Color }
		},
		.pipelineGroups = {
			{
				hl::PipelineInfo{
					.name = "comp",
					.shaderComp = "noop.comp",
					.bindPoint = hl::PipelineBindPoint::Compute
				}
			}
		}
	};

	REQUIRE_THROWS_AS(hl::passIsCompute(pass), std::runtime_error);
}

TEST_CASE("passIsCompute throws when a pass mixes graphics and compute", "[compute]")
{
	const hl::RenderpassInfo pass{
		.name = "mixed",
		.pipelineGroups = {
			{
				hl::PipelineInfo{
					.name = "gfx",
					.shaderVert = "a.vert",
					.shaderFrag = "a.frag",
					.bindPoint = hl::PipelineBindPoint::Graphics
				},
				hl::PipelineInfo{
					.name = "comp",
					.shaderComp = "noop.comp",
					.bindPoint = hl::PipelineBindPoint::Compute
				}
			}
		}
	};

	REQUIRE_THROWS_AS(hl::passIsCompute(pass), std::runtime_error);
}

TEST_CASE("generateDAG places a buffer consumer after a compute producer", "[compute]")
{
	const std::vector<hl::RenderpassInfo> passes{
		{
			.name = "compute_sim",
			.bufferOutputs = { "particles" },
			.pipelineGroups = {
				{
					hl::PipelineInfo{
						.name = "sim",
						.shaderComp = "sim.comp",
						.bindPoint = hl::PipelineBindPoint::Compute
					}
				}
			}
		},
		{
			.name = "draw",
			.inputs = { "particles" },
			.outputs = {
				hl::ResourceInfo{ .name = "color", .type = hl::ResourceType::Color }
			},
			.pipelineGroups = {
				{
					hl::PipelineInfo{
						.name = "draw_pipeline",
						.shaderVert = "a.vert",
						.shaderFrag = "a.frag"
					}
				}
			}
		}
	};

	const auto nodes = hl::RenderGraph::generateDAG(passes);
	REQUIRE(nodes.at("compute_sim").layer == 0);
	REQUIRE(nodes.at("draw").layer == 1);
}
