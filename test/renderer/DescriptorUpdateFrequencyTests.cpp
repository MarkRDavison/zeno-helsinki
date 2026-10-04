#include <catch2/catch_test_macros.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>

TEST_CASE("shouldWriteDescriptorBinding writes Static only before first update", "[descriptors]")
{
	REQUIRE(hl::shouldWriteDescriptorBinding(hl::DescriptorUpdateFrequency::Static, false));
	REQUIRE_FALSE(hl::shouldWriteDescriptorBinding(hl::DescriptorUpdateFrequency::Static, true));
}

TEST_CASE("shouldWriteDescriptorBinding always writes PerFrame", "[descriptors]")
{
	REQUIRE(hl::shouldWriteDescriptorBinding(hl::DescriptorUpdateFrequency::PerFrame, false));
	REQUIRE(hl::shouldWriteDescriptorBinding(hl::DescriptorUpdateFrequency::PerFrame, true));
}

TEST_CASE("DescriptorBinding defaults to PerFrame", "[descriptors]")
{
	const hl::DescriptorBinding binding{};
	REQUIRE(binding.updateFrequency == hl::DescriptorUpdateFrequency::PerFrame);
}
