#include <catch2/catch_test_macros.hpp>
#include <helsinki/System/Resource/Resource.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <helsinki/System/Resource/ResourceHandle.hpp>
#include <helsinki/System/Resource/LogicalResource.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>

namespace
{
	class TestResource : public hl::Resource
	{
	public:
		explicit TestResource(const std::string& id)
			: hl::Resource(id)
		{
		}
	};
}

TEST_CASE("LoadLogical loads children in definition order", "[LogicalResource]")
{
	hl::ResourceManager manager;
	std::vector<std::string> loadedOrder;

	const hl::ResourceDefinition definition
	{
		.name = "test_sheet",
		.type = "logical",
		.resources =
		{
			{ .name = "child_a", .type = "test" },
			{ .name = "child_b", .type = "test" },
			{ .name = "child_c", .type = "test" },
		}
	};

	const auto handle = manager.LoadLogical(definition, [&](const hl::ResourceDefinition::Child& child)
		{
			loadedOrder.push_back(child.name);
			manager.Load<TestResource>(child.name);
			return manager.HasResource<TestResource>(child.name);
		});

	REQUIRE(handle.IsValid());
	REQUIRE(loadedOrder == std::vector<std::string>{ "child_a", "child_b", "child_c" });

	const auto* logical = handle.Get();
	REQUIRE(logical != nullptr);
	REQUIRE(logical->GetChildren() == std::vector<std::string>{ "child_a", "child_b", "child_c" });
}

TEST_CASE("LoadLogical rolls back when loader fails", "[LogicalResource]")
{
	hl::ResourceManager manager;
	int loadAttempts = 0;

	const hl::ResourceDefinition definition
	{
		.name = "failing_sheet",
		.type = "logical",
		.resources =
		{
			{ .name = "child_a", .type = "test" },
			{ .name = "child_b", .type = "test" },
		}
	};

	const auto handle = manager.LoadLogical(definition, [&](const hl::ResourceDefinition::Child& child)
		{
			++loadAttempts;
			if (child.name == "child_a")
			{
				manager.Load<TestResource>(child.name);
				return true;
			}
			return false;
		});

	REQUIRE_FALSE(handle.IsValid());
	REQUIRE(loadAttempts == 2);
	REQUIRE_FALSE(manager.HasResource<TestResource>("child_a"));
	REQUIRE_FALSE(manager.HasResource<hl::LogicalResource>("failing_sheet"));
}

TEST_CASE("LoadLogical returns cached handle without re-invoking loader", "[LogicalResource]")
{
	hl::ResourceManager manager;
	int loaderCalls = 0;

	const hl::ResourceDefinition definition
	{
		.name = "cached_sheet",
		.type = "logical",
		.resources =
		{
			{ .name = "child_a", .type = "test" },
		}
	};

	const auto loader = [&](const hl::ResourceDefinition::Child& child)
		{
			++loaderCalls;
			manager.Load<TestResource>(child.name);
			return true;
		};

	const auto first = manager.LoadLogical(definition, loader);
	const auto second = manager.LoadLogical(definition, loader);

	REQUIRE(first.IsValid());
	REQUIRE(second.IsValid());
	REQUIRE(loaderCalls == 1);
}

TEST_CASE("LoadLogical cache ignores a later definition with extra children", "[LogicalResource]")
{
	hl::ResourceManager manager;

	const hl::ResourceDefinition first
	{
		.name = "shared_name_sheet",
		.type = "logical",
		.resources =
		{
			{ .name = "child_a", .type = "test" },
			{ .name = "child_b", .type = "test" },
		}
	};

	const hl::ResourceDefinition second
	{
		.name = "shared_name_sheet",
		.type = "logical",
		.resources =
		{
			{ .name = "child_a", .type = "test" },
			{ .name = "child_b", .type = "test" },
			{ .name = "child_c", .type = "test" },
		}
	};

	const auto loader = [&](const hl::ResourceDefinition::Child& child)
		{
			manager.Load<TestResource>(child.name);
			return true;
		};

	REQUIRE(manager.LoadLogical(first, loader).IsValid());
	REQUIRE(manager.LoadLogical(second, loader).IsValid());

	REQUIRE(manager.HasResource<TestResource>("child_a"));
	REQUIRE(manager.HasResource<TestResource>("child_b"));
	REQUIRE_FALSE(manager.HasResource<TestResource>("child_c"));
	REQUIRE(manager.GetResource<hl::LogicalResource>("shared_name_sheet")->GetChildren()
		== std::vector<std::string>{ "child_a", "child_b" });
}

TEST_CASE("Distinct logical resource names load independently", "[LogicalResource]")
{
	hl::ResourceManager manager;

	const auto loader = [&](const hl::ResourceDefinition::Child& child)
		{
			manager.Load<TestResource>(child.name);
			return true;
		};

	REQUIRE(manager.LoadLogical(
		{
			.name = "title_sheet",
			.type = "logical",
			.resources = { { .name = "child_a", .type = "test" } },
		},
		loader).IsValid());

	REQUIRE(manager.LoadLogical(
		{
			.name = "game_sheet",
			.type = "logical",
			.resources =
			{
				{ .name = "child_a", .type = "test" },
				{ .name = "child_c", .type = "test" },
			},
		},
		loader).IsValid());

	REQUIRE(manager.HasResource<TestResource>("child_a"));
	REQUIRE(manager.HasResource<TestResource>("child_c"));
	REQUIRE(manager.GetResource<hl::LogicalResource>("title_sheet")->GetChildren()
		== std::vector<std::string>{ "child_a" });
	REQUIRE(manager.GetResource<hl::LogicalResource>("game_sheet")->GetChildren()
		== std::vector<std::string>{ "child_a", "child_c" });
}

TEST_CASE("Unload releases logical resource children", "[LogicalResource]")
{
	hl::ResourceManager manager;

	const hl::ResourceDefinition definition
	{
		.name = "unload_sheet",
		.type = "logical",
		.resources =
		{
			{ .name = "child_a", .type = "test" },
			{ .name = "child_b", .type = "test" },
		}
	};

	const auto handle = manager.LoadLogical(definition, [&](const hl::ResourceDefinition::Child& child)
		{
			manager.Load<TestResource>(child.name);
			return true;
		});

	REQUIRE(handle.IsValid());
	REQUIRE(manager.HasResource<TestResource>("child_a"));
	REQUIRE(manager.HasResource<TestResource>("child_b"));

	manager.Release("unload_sheet");

	REQUIRE_FALSE(manager.HasResource<hl::LogicalResource>("unload_sheet"));
	REQUIRE_FALSE(manager.HasResource<TestResource>("child_a"));
	REQUIRE_FALSE(manager.HasResource<TestResource>("child_b"));
}
