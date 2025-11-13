#include <catch2/catch_test_macros.hpp>
#include <fakeit.hpp>
#include <unordered_map>
#include <vector>
#include <string>

namespace hl
{
	namespace test
	{
		struct RenderPassDef
		{
			std::string name;
			std::vector<std::string> inputs;
			std::vector<std::string> outputs;
		};

		struct RenderPassNode
		{
			const std::string name;
			const std::vector<RenderPassNode*> dependencies; // immutable

			RenderPassNode(std::string n, std::vector<RenderPassNode*> deps)
				: name(std::move(n)), dependencies(std::move(deps))
			{
			}
		}; 

		struct RenderPassState
		{
			bool executed = false;
		};

		class RenderGraph
		{
		public:
			RenderGraph() = default;

			void create(const std::vector<RenderPassDef>& definitions)
			{
				_definitions.clear();
				_frameState.clear();
				_nodes.clear();
				resetFrameState();

				// Insert definitions and initialize frame state
				for (const auto& def : definitions)
				{
					if (_definitions.contains(def.name))
					{
						throw std::runtime_error("Duplicate render pass definition.");
					}
					_definitions.insert({ def.name, def });
					_frameState[def.name] = RenderPassState{};
				}

				std::vector<RenderPassNode> tempNodes;
				tempNodes.reserve(definitions.size());

				for (const auto& defB : definitions)
				{
					std::vector<RenderPassNode*> deps;

					for (const auto& defA : definitions)
					{
						if (defA.name == defB.name)
						{
							continue;
						}

						for (const auto& inputB : defB.inputs)
						{
							for (const auto& outputA : defA.outputs)
							{
								if (inputB == outputA)
								{
									auto it = std::find_if(
										tempNodes.begin(), 
										tempNodes.end(),
										[&defA](const RenderPassNode& n) 
										{ 
											return n.name == defA.name; 
										});

									if (it != tempNodes.end())
									{
										deps.push_back(&(*it));
									}
									break;
								}
							}
						}
					}

					tempNodes.emplace_back(defB.name, std::move(deps));
				}

				_nodes = std::move(tempNodes);
			}

			std::vector<const RenderPassNode*> getReadyPasses() const
			{
				std::vector<const RenderPassNode*> ready;

				for (const auto& node : _nodes)
				{
					if (isExecuted(node.name))
					{
						continue;
					}

					bool depsSatisfied = true;
					for (auto* dep : node.dependencies)
					{
						if (!isExecuted(dep->name))
						{
							depsSatisfied = false;
							break;
						}
					}

					if (depsSatisfied)
					{
						ready.push_back(&node);
					}
				}

				return ready;
			}

			void resetFrameState()
			{
				for (auto& kv : _frameState)
				{
					kv.second.executed = false;
				}
			}

			std::vector<std::string> getNodeNames() const
			{
				std::vector<std::string> names;

				for (const auto& node : _nodes)
				{
					names.push_back(node.name);
				}

				return names;
			}

			const std::vector<RenderPassNode>& getNodes() const { return _nodes; }

			bool isExecuted(const std::string& name) const
			{
				auto it = _frameState.find(name);
				return it != _frameState.end() && it->second.executed;
			}

			void markExecuted(const std::string& name)
			{
				auto it = _frameState.find(name);
				if (it != _frameState.end())
				{
					it->second.executed = true;
				}
			}

			const RenderPassDef& getDefinition(const std::string& name) const
			{
				auto it = _definitions.find(name);
				if (it == _definitions.end())
				{
					throw std::runtime_error("Render pass definition not found: " + name);
				}

				return it->second;
			}

		private:
			std::unordered_map<std::string, RenderPassDef> _definitions;
			std::vector<RenderPassNode> _nodes;
			std::unordered_map<std::string, RenderPassState> _frameState;

		};

		TEST_CASE("Immutable RenderGraph DAG with accessors", "[DAG]")
		{
			std::vector<RenderPassDef> defs = {
				{.name = "scene_pass", .inputs = {}, .outputs = {"scene_color"} },
				{.name = "postprocess_pass", .inputs = {"scene_color"}, .outputs = {"post_color"} },
				{.name = "ui_pass", .inputs = {}, .outputs = {"ui_color"} },
				{.name = "final_composite_pass", .inputs = {"post_color", "ui_color"}, .outputs = {"swapchain"} }
			};

			RenderGraph graph;
			graph.create(defs);

			const auto& nodes = graph.getNodes();

			// Helper map for easy lookup
			std::unordered_map<std::string, const RenderPassNode*> nodeMap;
			for (const auto& node : nodes)
			{
				nodeMap[node.name] = &node;
			}

			const auto* sceneNode = nodeMap["scene_pass"];
			const auto* postNode = nodeMap["postprocess_pass"];
			const auto* uiNode = nodeMap["ui_pass"];
			const auto* finalNode = nodeMap["final_composite_pass"];

			REQUIRE(sceneNode->dependencies.empty());
			REQUIRE(postNode->dependencies.size() == 1);
			REQUIRE(postNode->dependencies[0]->name == "scene_pass");
			REQUIRE(uiNode->dependencies.empty());

			std::vector<std::string> finalDeps;
			for (auto* dep : finalNode->dependencies)
				finalDeps.push_back(dep->name);

			REQUIRE(finalDeps.size() == 2);
			REQUIRE((finalDeps[0] == "postprocess_pass" || finalDeps[0] == "ui_pass"));
			REQUIRE((finalDeps[1] == "postprocess_pass" || finalDeps[1] == "ui_pass"));
		}

		TEST_CASE("RenderGraph getReadyPasses", "[DAG]")
		{
			std::vector<RenderPassDef> defs = {
				{.name = "scene_pass", .inputs = {}, .outputs = {"scene_color"} },
				{.name = "postprocess_pass", .inputs = {"scene_color"}, .outputs = {"post_color"} },
				{.name = "ui_pass", .inputs = {}, .outputs = {"ui_color"} },
				{.name = "final_composite_pass", .inputs = {"post_color", "ui_color"}, .outputs = {"swapchain"} }
			};

			RenderGraph graph;
			graph.create(defs);
			graph.resetFrameState();

			// At the start, passes with no dependencies should be ready
			auto ready = graph.getReadyPasses();
			std::vector<std::string> readyNames;
			for (auto* pass : ready)
			{
				readyNames.push_back(pass->name);
			}

			REQUIRE(readyNames.size() == 2);
			REQUIRE((readyNames[0] == "scene_pass" || readyNames[0] == "ui_pass"));
			REQUIRE((readyNames[1] == "scene_pass" || readyNames[1] == "ui_pass"));

			// Execute scene_pass
			graph.markExecuted("scene_pass");

			ready = graph.getReadyPasses();
			readyNames.clear();
			for (auto* pass : ready)
			{
				readyNames.push_back(pass->name);
			}

			// postprocess_pass depends on scene_pass, ui_pass is independent
			REQUIRE(readyNames.size() == 2);
			REQUIRE((readyNames[0] == "postprocess_pass" || readyNames[0] == "ui_pass"));
			REQUIRE((readyNames[1] == "postprocess_pass" || readyNames[1] == "ui_pass"));

			// Execute remaining passes
			graph.markExecuted("ui_pass");
			graph.markExecuted("postprocess_pass");

			ready = graph.getReadyPasses();
			readyNames.clear();
			for (auto* pass : ready)
			{
				readyNames.push_back(pass->name);
			}

			// Only final_composite_pass should be ready now
			REQUIRE(readyNames.size() == 1);
			REQUIRE(readyNames[0] == "final_composite_pass");

			// After executing final_composite_pass, no ready passes remain
			graph.markExecuted("final_composite_pass");
			ready = graph.getReadyPasses();
			REQUIRE(ready.empty());
		}
	}
}