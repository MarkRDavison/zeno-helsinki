#include <catch2/catch_test_macros.hpp>
#include <fakeit.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphRenderpassResources.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>

namespace hl
{

	namespace tests
	{
        std::vector<hl::RenderpassInfo> createTestsRenderpasses()
        {
            std::vector<hl::RenderpassInfo> renderpasses =
            {
                hl::RenderpassInfo
                {
                    .name = "scene_pass",
                    .useMultiSampling = true,
                    .inputs = {},
                    .outputs =
                    {
                        hl::ResourceInfo
                        {
                            .name = "scene_color",
                            .type = hl::ResourceType::Color,
                            .format = "VK_FORMAT_B8G8R8A8_SRGB"
                        },
                        hl::ResourceInfo
                        {
                            .name = "scene_depth",
                            .type = hl::ResourceType::Depth,
                            .format = "VK_FORMAT_D32_SFLOAT"
                        }
                    },
                    .pipelineGroups =
                    {
                        {
                            hl::PipelineInfo
                        {
                            .name = "skybox_pipeline",
                            .shaderVert = ("/data/shaders/skybox.vert"),
                            .shaderFrag = ("/data/shaders/skybox.frag"),
                            .descriptorSets =
                            {
                                hl::DescriptorSetInfo
                                {
                                    .name = "",
                                    .bindings =
                                    {
                                        hl::DescriptorBinding
                                        {
                                            .binding = 0,
                                            .type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
                                            .stage = "VERTEX",
                                            .resource = "model_matrix_ubo"
                                        },
                                        hl::DescriptorBinding
                                        {
                                            .binding = 1,
                                            .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                            .stage = "FRAGMENT",
                                            .resource = "skybox_texture"
                                        }
                                    }
                                }
                            },
                            .depthState =
                            {
                                .writeEnable = false,
                                .compareOp = VK_COMPARE_OP_LESS_OR_EQUAL
                            },
                            .rasterState =
                            {
                                .cullMode = VK_CULL_MODE_NONE
                            },
                            .enableBlending = false
                        },
                        hl::PipelineInfo
                        {
                            .name = "model_pipeline",
                            .shaderVert = ("/data/shaders/triangle.vert"),
                            .shaderFrag = ("/data/shaders/triangle.frag"),
                            .descriptorSets =
                            {
                                hl::DescriptorSetInfo
                                {
                                    .name = "model_uniforms",
                                    .bindings =
                                    {
                                        hl::DescriptorBinding
                                        {
                                            .binding = 0,
                                            .type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
                                            .stage = "VERTEX",
                                            .resource = "model_matrix_ubo"
                                        },
                                        hl::DescriptorBinding
                                        {
                                            .binding = 1,
                                            .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                            .stage = "FRAGMENT",
                                            .resource = "viking_room"
                                        }
                                    }
                                }
                            },
                            .vertexInputInfo = hl::VertexInputInfo
                            {
                                .attributes =
                                {
                                    {
                                        .name = "inPosition",
                                        .format = hl::VertexAttributeFormat::Vec3,
                                        .location = 0,
                                        .offset = 0
                                    },
                                    {
                                        .name = "inColor",
                                        .format = hl::VertexAttributeFormat::Vec3,
                                        .location = 1,
                                        .offset = 0
                                    },
                                    {
                                        .name = "inTexCoord",
                                        .format = hl::VertexAttributeFormat::Vec2,
                                        .location = 2,
                                        .offset = 0
                                    },
                                },
                                .stride = sizeof(float) * (3 + 3 + 2)
                            },
                            .rasterState =
                            {
                                .cullMode = VK_CULL_MODE_BACK_BIT
                            },
                            .enableBlending = false
                        }
                        }
                    }
                },
                hl::RenderpassInfo
                {
                    .name = "postprocess_pass",
                    .useMultiSampling = false,
                    .inputs = { "scene_color" },
                    .outputs =
                    {
                        hl::ResourceInfo
                        {
                            .name = "post_color",
                            .type = hl::ResourceType::Color,
                            .format = "VK_FORMAT_B8G8R8A8_SRGB"
                        }
                    },
                    .pipelineGroups =
                    {
                        {
                            hl::PipelineInfo
                            {
                                .name = "postprocess_pipeline",
                                .shaderVert = ("/data/shaders/post_process.vert"),
                                .shaderFrag = ("/data/shaders/post_process.frag"),
                                .descriptorSets =
                                {
                                    hl::DescriptorSetInfo
                                    {
                                        .name = "input_sampler",
                                        .bindings =
                                        {
                                            hl::DescriptorBinding
                                            {
                                                .binding = 0,
                                                .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                                .stage = "FRAGMENT",
                                                .resource = "scene_color"
                                            }
                                        }
                                    }
                                },
                                .depthState =
                                {
                                    .testEnable = false,
                                    .writeEnable = false
                                },
                                .rasterState =
                                {
                                    .cullMode = VK_CULL_MODE_NONE
                                },
                                .enableBlending = false
                            }
                        }
                    }
                },
                hl::RenderpassInfo
                {
                    .name = "ui_pass",
                    .useMultiSampling = false,
                    .inputs = {},
                    .outputs =
                    {
                        hl::ResourceInfo
                        {
                            .name = "ui_color",
                            .type = hl::ResourceType::Color,
                            .format = "VK_FORMAT_B8G8R8A8_SRGB",
                            .clear = VkClearValue{.color = {{ 0.0f, 0.0f, 0.0f, 0.0f }}}
                        }
                    },
                    .pipelineGroups =
                    {
                        {
                            hl::PipelineInfo
                            {
                                .name = "ui",
                                .shaderVert = ("/data/shaders/ui.vert"),
                                .shaderFrag = ("/data/shaders/ui.frag"),
                                .descriptorSets = {},
                                .depthState =
                                {
                                    .testEnable = false,
                                    .writeEnable = false
                                },
                                .rasterState =
                                {
                                    .cullMode = VK_CULL_MODE_NONE
                                },
                                .enableBlending = true // keep blending for UI elements
                            }
                        }
                    }
                },
                hl::RenderpassInfo
                {
                    .name = "composite_pass",
                    .useMultiSampling = false,
                    .inputs = { "post_color", "ui_color" },
                    .outputs =
                    {
                        hl::ResourceInfo
                        {
                            .name = "swapchain_color",
                            .type = hl::ResourceType::Color,
                            .format = "VK_FORMAT_B8G8R8A8_SRGB"
                        }
                    },
                    .pipelineGroups =
                    {
                        {
                            hl::PipelineInfo
                            {
                                .name = "composite_pipeline",
                                .shaderVert = ("/data/shaders/fullscreen_sample.vert"),
                                .shaderFrag = ("/data/shaders/composite.frag"),
                                .descriptorSets =
                                {
                                    hl::DescriptorSetInfo
                                    {
                                        .name = "composite_inputs",
                                        .bindings =
                                        {
                                            hl::DescriptorBinding
                                            {
                                                .binding = 0,
                                                .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                                .stage = "FRAGMENT",
                                                .resource = "post_color"
                                            },
                                            hl::DescriptorBinding
                                            {
                                                .binding = 1,
                                                .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                                .stage = "FRAGMENT",
                                                .resource = "ui_color"
                                            }
                                        }
                                    }
                                },
                                .depthState =
                                {
                                    .testEnable = false,
                                    .writeEnable = false
                                },
                                .rasterState =
                                {
                                    .cullMode = VK_CULL_MODE_NONE
                                },
                                .enableBlending = true
                            }
                        }
                    }
                }
            };

            return renderpasses;
        }
        
		TEST_CASE("Renderpass info can generate DAG", "[RenderPassInfo]")
		{
            const auto& renderpasses = createTestsRenderpasses();

            auto nodes = RenderGraph::generateDAG(renderpasses);

            std::unordered_map<uint32_t, int> counts;
            for (auto& [_, n] : nodes) 
            { 
                counts[n.layer]++; 
            }

            REQUIRE(nodes["scene_pass"].layer == 0);
            REQUIRE(nodes["ui_pass"].layer == 0);

            REQUIRE(nodes["postprocess_pass"].layer == 1);

            REQUIRE(nodes["composite_pass"].layer == 2);

            REQUIRE(nodes["scene_pass"].layer < nodes["postprocess_pass"].layer);
            REQUIRE(nodes["ui_pass"].layer < nodes["composite_pass"].layer);
            REQUIRE(nodes["postprocess_pass"].layer < nodes["composite_pass"].layer);


            REQUIRE(counts[0] == 2); // scene_pass, ui_pass  
            REQUIRE(counts[1] == 1); // postprocess_pass  
            REQUIRE(counts[2] == 1); // composite_pass  
		}

        TEST_CASE("RenderGraph detects cycles", "[RenderPassInfo]")
        {
            // A → B → C → A
            std::vector<hl::RenderpassInfo> renderpasses;

            {
                hl::RenderpassInfo A;
                A.name = "A";
                A.inputs = { "C_out" };      // depends on C
                A.outputs = { { "A_out" } };
                renderpasses.push_back(A);
            }
            {
                hl::RenderpassInfo B;
                B.name = "B";
                B.inputs = { "A_out" };      // depends on A
                B.outputs = { { "B_out" } };
                renderpasses.push_back(B);
            }
            {
                hl::RenderpassInfo C;
                C.name = "C";
                C.inputs = { "B_out" };      // depends on B
                C.outputs = { { "C_out" } };
                renderpasses.push_back(C);
            }

            REQUIRE_THROWS_AS(
                RenderGraph::generateDAG(renderpasses),
                std::runtime_error);
        }

        TEST_CASE("RenderGraph detects disconnected branches", "[RenderPassInfo]")
        {
            std::vector<hl::RenderpassInfo> renderpasses;

            renderpasses.push_back(
                hl::RenderpassInfo{
                    .name = "A",
                    .inputs = {},
                    .outputs = { hl::ResourceInfo{.name = "A_out" } }
                });
            renderpasses.push_back(
                hl::RenderpassInfo{
                    .name = "B",
                    .inputs = { "A_out" },
                    .outputs = { hl::ResourceInfo{.name = "B_out" } }
                });
            renderpasses.push_back(
                hl::RenderpassInfo{
                    .name = "C",
                    .inputs = {},
                    .outputs = { hl::ResourceInfo{.name = "C_out" } }
                });
            renderpasses.push_back(
                hl::RenderpassInfo{
                    .name = "D",
                    .inputs = { "C_out" },
                    .outputs = { hl::ResourceInfo{.name = "D_out" } }
                });

            REQUIRE_THROWS_AS(RenderGraph::generateDAG(renderpasses),
                std::runtime_error);
        }

        TEST_CASE("RenderGraph handles diamond dependency", "[RenderPassInfo]")
        {
            std::vector<hl::RenderpassInfo> renderpasses;

            renderpasses.push_back(
                hl::RenderpassInfo{
                    .name = "A",
                    .inputs = {},
                    .outputs = { hl::ResourceInfo{.name = "A_out" } }
                });

            renderpasses.push_back(
                hl::RenderpassInfo{
                    .name = "B",
                    .inputs = { "A_out" },
                    .outputs = { hl::ResourceInfo{.name = "B_out" } }
                });
            renderpasses.push_back(
                hl::RenderpassInfo{
                    .name = "C",
                    .inputs = { "A_out" },
                    .outputs = { hl::ResourceInfo{.name = "C_out" } }
                });

            renderpasses.push_back(
                hl::RenderpassInfo{
                    .name = "D",
                    .inputs = { "B_out", "C_out" },
                    .outputs = { hl::ResourceInfo{.name = "D_out" } }
                });

            auto nodes = RenderGraph::generateDAG(renderpasses);

            REQUIRE(nodes["A"].layer == 0);
            REQUIRE(nodes["B"].layer == 1);
            REQUIRE(nodes["C"].layer == 1);
            REQUIRE(nodes["D"].layer == 2);

            uint32_t maxLayer = 0;
            for (auto& [_, node] : nodes)
            {
                if (node.layer > maxLayer)
                {
                    maxLayer = node.layer;
                }
            }

            size_t topLayerCount = 0;
            for (auto& [_, node] : nodes)
            {
                if (node.layer == maxLayer)
                {
                    topLayerCount++;
                }
            }

            REQUIRE(topLayerCount == 1);
        }

        TEST_CASE("RenderGraph detects self-cycle", "[RenderPassInfo]")
        {
            std::vector<hl::RenderpassInfo> renderpasses;

            renderpasses.push_back(
                hl::RenderpassInfo{
                    .name = "A",
                    .inputs = { "A_out" },
                    .outputs = { hl::ResourceInfo{.name = "A_out" } }
                });

            REQUIRE_THROWS_AS(RenderGraph::generateDAG(renderpasses),
                std::runtime_error);
        }

        TEST_CASE("RenderGraph handles long linear chain", "[RenderPassInfo]")
        {
            std::vector<hl::RenderpassInfo> renderpasses;

            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "A", .inputs = {}, .outputs = { hl::ResourceInfo{.name = "A_out" } } });
            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "B", .inputs = { "A_out" }, .outputs = { hl::ResourceInfo{.name = "B_out" } } });
            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "C", .inputs = { "B_out" }, .outputs = { hl::ResourceInfo{.name = "C_out" } } });
            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "D", .inputs = { "C_out" }, .outputs = { hl::ResourceInfo{.name = "D_out" } } });
            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "E", .inputs = { "D_out" }, .outputs = { hl::ResourceInfo{.name = "swapchain_color" } } });

            auto nodes = RenderGraph::generateDAG(renderpasses);

            REQUIRE(nodes["A"].layer == 0);
            REQUIRE(nodes["B"].layer == 1);
            REQUIRE(nodes["C"].layer == 2);
            REQUIRE(nodes["D"].layer == 3);
            REQUIRE(nodes["E"].layer == 4);

            uint32_t maxLayer = 0;
            for (auto& [_, node] : nodes)
            {
                if (node.layer > maxLayer)
                {
                    maxLayer = node.layer;
                }
            }

            size_t topLayerCount = 0;
            for (auto& [_, node] : nodes)
            {
                if (node.layer == maxLayer)
                {
                    topLayerCount++;
                }
            }

            REQUIRE(topLayerCount == 1);
        }

        TEST_CASE("RenderGraph handles parallel branches with varying depth", "[RenderPassInfo]")
        {
            std::vector<hl::RenderpassInfo> renderpasses;

            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "A", .inputs = {}, .outputs = { hl::ResourceInfo{.name = "A_out" } } });
            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "B", .inputs = { "A_out" }, .outputs = { hl::ResourceInfo{.name = "B_out" } } });
            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "E", .inputs = { "B_out" }, .outputs = { hl::ResourceInfo{.name = "E_out" } } });

            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "C", .inputs = {}, .outputs = { hl::ResourceInfo{.name = "C_out" } } });
            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "D", .inputs = { "C_out" }, .outputs = { hl::ResourceInfo{.name = "D_out" } } });
            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "F", .inputs = { "D_out" }, .outputs = { hl::ResourceInfo{.name = "F_out" } } });

            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "G", .inputs = { "E_out", "F_out" }, .outputs = { hl::ResourceInfo{.name = "swapchain_color" } } });

            auto nodes = RenderGraph::generateDAG(renderpasses);

            REQUIRE(nodes["A"].layer == 0);
            REQUIRE(nodes["B"].layer == 1);
            REQUIRE(nodes["E"].layer == 2);

            REQUIRE(nodes["C"].layer == 0);
            REQUIRE(nodes["D"].layer == 1);
            REQUIRE(nodes["F"].layer == 2);

            REQUIRE(nodes["G"].layer == 3);

            uint32_t maxLayer = 0;
            for (auto& [_, node] : nodes)
            {
                if (node.layer > maxLayer)
                {
                    maxLayer = node.layer;
                }
            }

            size_t topLayerCount = 0;
            for (auto& [_, node] : nodes)
            {
                if (node.layer == maxLayer)
                {
                    topLayerCount++;
                }
            }

            REQUIRE(topLayerCount == 1);
        }

        TEST_CASE("RenderGraph handles multi-merge DAG with intermediate convergence", "[RenderPassInfo]")
        {
            std::vector<hl::RenderpassInfo> renderpasses;

            /*
                A
               / \
              B   C
               \ / \
                D   E
                 \ /
                  F
                  |
                  G (final, swapchain)
            */

            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "A", .inputs = {}, .outputs = { hl::ResourceInfo{.name = "A_out" } } });

            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "B", .inputs = { "A_out" }, .outputs = { hl::ResourceInfo{.name = "B_out" } } });
            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "C", .inputs = { "A_out" }, .outputs = { hl::ResourceInfo{.name = "C_out" } } });

            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "D", .inputs = { "B_out", "C_out" }, .outputs = { hl::ResourceInfo{.name = "D_out" } } });
            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "E", .inputs = { "C_out" }, .outputs = { hl::ResourceInfo{.name = "E_out" } } });

            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "F", .inputs = { "D_out", "E_out" }, .outputs = { hl::ResourceInfo{.name = "F_out" } } });

            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "G", .inputs = { "F_out" }, .outputs = { hl::ResourceInfo{.name = "swapchain_color" } } });

            auto nodes = RenderGraph::generateDAG(renderpasses);

            REQUIRE(nodes["A"].layer == 0);
            REQUIRE(nodes["B"].layer == 1);
            REQUIRE(nodes["C"].layer == 1);
            REQUIRE(nodes["D"].layer == 2);
            REQUIRE(nodes["E"].layer == 2);
            REQUIRE(nodes["F"].layer == 3);
            REQUIRE(nodes["G"].layer == 4);

            uint32_t maxLayer = 0;
            for (auto& [_, node] : nodes)
            {
                if (node.layer > maxLayer)
                {
                    maxLayer = node.layer;
                }
            }

            size_t topLayerCount = 0;
            for (auto& [_, node] : nodes)
            {
                if (node.layer == maxLayer)
                {
                    topLayerCount++;
                }
            }

            REQUIRE(topLayerCount == 1);
        }

        TEST_CASE("RenderGraph handles deeply nested diamond pattern", "[RenderPassInfo]")
        {
            std::vector<hl::RenderpassInfo> renderpasses;

            /*
                    A
                   / \
                  B   C
                 / \ / \
                D   E   F
                 \ / \ /
                  G   H
                   \ /
                    I (final, swapchain)
            */

            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "A", .inputs = {}, .outputs = { hl::ResourceInfo{.name = "A_out" } } });

            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "B", .inputs = { "A_out" }, .outputs = { hl::ResourceInfo{.name = "B_out" } } });
            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "C", .inputs = { "A_out" }, .outputs = { hl::ResourceInfo{.name = "C_out" } } });

            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "D", .inputs = { "B_out" }, .outputs = { hl::ResourceInfo{.name = "D_out" } } });
            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "E", .inputs = { "B_out", "C_out" }, .outputs = { hl::ResourceInfo{.name = "E_out" } } });
            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "F", .inputs = { "C_out" }, .outputs = { hl::ResourceInfo{.name = "F_out" } } });

            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "G", .inputs = { "D_out", "E_out" }, .outputs = { hl::ResourceInfo{.name = "G_out" } } });
            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "H", .inputs = { "E_out", "F_out" }, .outputs = { hl::ResourceInfo{.name = "H_out" } } });

            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "I", .inputs = { "G_out", "H_out" }, .outputs = { hl::ResourceInfo{.name = "swapchain_color" } } });

            auto nodes = RenderGraph::generateDAG(renderpasses);

            REQUIRE(nodes["A"].layer == 0);
            REQUIRE(nodes["B"].layer == 1);
            REQUIRE(nodes["C"].layer == 1);
            REQUIRE(nodes["D"].layer == 2);
            REQUIRE(nodes["E"].layer == 2);
            REQUIRE(nodes["F"].layer == 2);
            REQUIRE(nodes["G"].layer == 3);
            REQUIRE(nodes["H"].layer == 3);
            REQUIRE(nodes["I"].layer == 4);

            uint32_t maxLayer = 0;
            for (auto& [_, node] : nodes)
            {
                if (node.layer > maxLayer)
                {
                    maxLayer = node.layer;
                }
            }

            size_t topLayerCount = 0;
            for (auto& [_, node] : nodes)
            {
                if (node.layer == maxLayer)
                {
                    topLayerCount++;
                }
            }

            REQUIRE(topLayerCount == 1);
        }

        TEST_CASE("RenderGraph handles many parallel chains converging to final node", "[RenderPassInfo]")
        {
            const int numChains = 10;
            const int chainLength = 5;
            std::vector<hl::RenderpassInfo> renderpasses;

            for (int c = 0; c < numChains; c++)
            {
                std::string prevOutput;
                for (int i = 0; i < chainLength; i++)
                {
                    std::string nodeName = "Chain" + std::to_string(c) + "_Node" + std::to_string(i);
                    std::string outputName = nodeName + "_out";

                    hl::RenderpassInfo pass{
                        .name = nodeName,
                        .inputs = prevOutput.empty() ? std::vector<std::string>{} : std::vector<std::string>{ prevOutput },
                        .outputs = { hl::ResourceInfo{.name = outputName } }
                    };

                    renderpasses.push_back(pass);
                    prevOutput = outputName;
                }
            }

            std::vector<std::string> finalInputs;
            for (int c = 0; c < numChains; c++)
            {
                finalInputs.push_back("Chain" + std::to_string(c) + "_Node" + std::to_string(chainLength - 1) + "_out");
            }

            renderpasses.push_back(
                hl::RenderpassInfo{ .name = "Final", .inputs = finalInputs, .outputs = { hl::ResourceInfo{.name = "swapchain_color" } } });

            auto nodes = RenderGraph::generateDAG(renderpasses);

            uint32_t maxLayer = 0;
            for (auto& [_, node] : nodes)
            {
                if (node.layer > maxLayer)
                {
                    maxLayer = node.layer;
                }
            }

            size_t topLayerCount = 0;
            for (auto& [_, node] : nodes)
            {
                if (node.layer == maxLayer)
                {
                    topLayerCount++;
                }
            }

            REQUIRE(topLayerCount == 1);
            REQUIRE(nodes["Final"].layer == maxLayer);

            // Check that chain nodes have increasing layers
            for (int c = 0; c < numChains; c++)
            {
                for (int i = 0; i < chainLength; i++)
                {
                    std::string nodeName = "Chain" + std::to_string(c) + "_Node" + std::to_string(i);
                    REQUIRE(nodes[nodeName].layer == i);
                }
            }
        }

        TEST_CASE("RenderGraph detects cycles in complex DAG", "[RenderPassInfo]")
        {
            std::vector<hl::RenderpassInfo> renderpasses;

            renderpasses.push_back({ .name = "A", .inputs = {}, .outputs = { hl::ResourceInfo{.name = "A_out" } } });
            renderpasses.push_back({ .name = "B", .inputs = { "A_out" }, .outputs = { hl::ResourceInfo{.name = "B_out" } } });
            renderpasses.push_back({ .name = "C", .inputs = { "B_out" }, .outputs = { hl::ResourceInfo{.name = "C_out" } } });
            renderpasses.push_back({ .name = "D", .inputs = { "C_out" }, .outputs = { hl::ResourceInfo{.name = "D_out" } } });
            renderpasses.push_back({ .name = "E", .inputs = { "D_out" }, .outputs = { hl::ResourceInfo{.name = "E_out" } } });

            renderpasses.push_back({ .name = "F", .inputs = {}, .outputs = { hl::ResourceInfo{.name = "F_out" } } });
            renderpasses.push_back({ .name = "G", .inputs = { "F_out" }, .outputs = { hl::ResourceInfo{.name = "G_out" } } });
            renderpasses.push_back({ .name = "H", .inputs = { "G_out" }, .outputs = { hl::ResourceInfo{.name = "H_out" } } });

            renderpasses.push_back({
                .name = "Final",
                .inputs = { "E_out", "H_out" },
                .outputs = { hl::ResourceInfo{.name = "swapchain_color" } }
                });

            // Introduce a cycle: D → B
            renderpasses.push_back({
                .name = "CycleHelper",
                .inputs = { "D_out" },
                .outputs = { hl::ResourceInfo{.name = "B_out" } }
                });

            REQUIRE_THROWS_AS(RenderGraph::generateDAG(renderpasses), std::runtime_error);
        }

	}
}