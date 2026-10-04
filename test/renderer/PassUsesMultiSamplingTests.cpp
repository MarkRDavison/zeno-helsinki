#include <catch2/catch_test_macros.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>

TEST_CASE("passUsesMultiSampling is false when all outputs omit the flag", "[msaa]")
{
	const hl::RenderpassInfo pass{
		.name = "one_sample",
		.outputs = {
			{ .name = "color", .type = hl::ResourceType::Color, .format = "VK_FORMAT_B8G8R8A8_SRGB" },
		},
	};
	REQUIRE_FALSE(hl::passUsesMultiSampling(pass));
}

TEST_CASE("passUsesMultiSampling is true when all outputs opt in", "[msaa]")
{
	const hl::RenderpassInfo pass{
		.name = "msaa",
		.outputs = {
			{ .name = "color", .type = hl::ResourceType::Color, .format = "VK_FORMAT_B8G8R8A8_SRGB", .useMultiSampling = true },
			{ .name = "depth", .type = hl::ResourceType::Depth, .format = "VK_FORMAT_D32_SFLOAT", .useMultiSampling = true },
		},
	};
	REQUIRE(hl::passUsesMultiSampling(pass));
}

TEST_CASE("passUsesMultiSampling throws when outputs disagree", "[msaa]")
{
	const hl::RenderpassInfo pass{
		.name = "mixed",
		.outputs = {
			{ .name = "color", .type = hl::ResourceType::Color, .format = "VK_FORMAT_B8G8R8A8_SRGB", .useMultiSampling = true },
			{ .name = "depth", .type = hl::ResourceType::Depth, .format = "VK_FORMAT_D32_SFLOAT" },
		},
	};
	REQUIRE_THROWS_AS(hl::passUsesMultiSampling(pass), std::runtime_error);
}

TEST_CASE("passUsesMultiSampling is false when there are no outputs", "[msaa]")
{
	const hl::RenderpassInfo pass{ .name = "empty" };
	REQUIRE_FALSE(hl::passUsesMultiSampling(pass));
}
