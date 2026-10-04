#include <catch2/catch_test_macros.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>

namespace
{
	hl::DescriptorBinding binding(
		uint32_t index,
		const std::string& type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
		const std::string& stage = "VERTEX",
		uint32_t count = 1)
	{
		return hl::DescriptorBinding{
			.binding = index,
			.type = type,
			.stage = stage,
			.count = count,
		};
	}
}

TEST_CASE("descriptorSetLayoutsCompatible accepts identical layout fields", "[descriptors]")
{
	const std::vector<hl::DescriptorSetInfo> a{
		{
			.name = "set_a",
			.bindings = {
				binding(0),
				binding(1, "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER", "FRAGMENT", 64),
			},
		},
	};
	auto b = a;
	b[0].name = "set_b";
	b[0].bindings[0].resource = "camera";
	b[0].bindings[0].updateFrequency = hl::DescriptorUpdateFrequency::Static;
	b[0].bindings[1].resource = "other_atlas";
	b[0].bindings[1].updateFrequency = hl::DescriptorUpdateFrequency::PerFrame;

	REQUIRE(hl::descriptorSetLayoutsCompatible(a, b));
}

TEST_CASE("descriptorSetLayoutsCompatible fails on type mismatch", "[descriptors]")
{
	const std::vector<hl::DescriptorSetInfo> a{ { .name = "s", .bindings = { binding(0) } } };
	auto b = a;
	b[0].bindings[0].type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER";

	REQUIRE_FALSE(hl::descriptorSetLayoutsCompatible(a, b));
}

TEST_CASE("descriptorSetLayoutsCompatible fails on binding count mismatch", "[descriptors]")
{
	const std::vector<hl::DescriptorSetInfo> a{
		{ .name = "s", .bindings = { binding(0), binding(1) } },
	};
	const std::vector<hl::DescriptorSetInfo> b{
		{ .name = "s", .bindings = { binding(0) } },
	};

	REQUIRE_FALSE(hl::descriptorSetLayoutsCompatible(a, b));
}

TEST_CASE("descriptorSetLayoutsCompatible fails empty vs non-empty", "[descriptors]")
{
	const std::vector<hl::DescriptorSetInfo> empty{};
	const std::vector<hl::DescriptorSetInfo> nonempty{
		{ .name = "s", .bindings = { binding(0) } },
	};

	REQUIRE_FALSE(hl::descriptorSetLayoutsCompatible(empty, nonempty));
	REQUIRE(hl::descriptorSetLayoutsCompatible(empty, empty));
}
