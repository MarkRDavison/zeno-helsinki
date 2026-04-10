#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/TextPushConstantObject.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Renderer/Resource/WellKnownResources.hpp>

namespace hl
{

	RenderpassInfo RenderGraphHelpers::createTextRenderpassInfo(
        const std::string& cameraMatrixResourceId)
	{
		return RenderpassInfo
		{
            .name = "text_pass",
            .useMultiSampling = false,
            .inputs = {},
            .outputs =
            {
                hl::ResourceInfo
                {
                    .name = "text_color",
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
                        .name = "text_pipeline",
                        .shaderVert = WellKnownResources::TextVertexShader_Name,
                        .shaderFrag = WellKnownResources::TextFragmentShader_Name,
                        .descriptorSets =
                        {
                            hl::DescriptorSetInfo
                            {
                                .bindings =
                                {
                                    hl::DescriptorBinding
                                    {
                                        .binding = 0,
                                        .type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
                                        .stage = "VERTEX",
                                        .resource = cameraMatrixResourceId
                                    },
                                    hl::DescriptorBinding
                                    {
                                        .binding = 1,
                                        .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                        .stage = "VERTEX&FRAGMENT",
                                        .count = 64
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
                                    .format = hl::VertexAttributeFormat::Vec2,
                                    .location = 0,
                                    .offset = offsetof(hl::Vertex22D, pos)
                                },
                                {
                                    .name = "inTexCoord",
                                    .format = hl::VertexAttributeFormat::Vec2,
                                    .location = 1,
                                    .offset = offsetof(hl::Vertex22D, texCoord)
                                }
                            },
                            .stride = sizeof(hl::Vertex22D)
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
                        .enableBlending = true,
                        .pushConstantSize = sizeof(hl::TextPushConstantObject)
                    },
                    hl::PipelineInfo
                    {
                        .name = "sdf_text_pipeline",
                        .shaderVert = WellKnownResources::TextVertexShader_Name,
                        .shaderFrag = WellKnownResources::SdfFragmentShader_Name,
                        .descriptorSets =
                        {
                            hl::DescriptorSetInfo
                            {
                                .bindings =
                                {
                                    hl::DescriptorBinding
                                    {
                                        .binding = 0,
                                        .type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
                                        .stage = "VERTEX",
                                        .resource = cameraMatrixResourceId
                                    },
                                    hl::DescriptorBinding
                                    {
                                        .binding = 1,
                                        .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                        .stage = "VERTEX&FRAGMENT",
                                        .count = 64 // TODO: Constant? MAX_FONTS????
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
                                    .format = hl::VertexAttributeFormat::Vec2,
                                    .location = 0,
                                    .offset = offsetof(hl::Vertex22D, pos)
                                },
                                {
                                    .name = "inTexCoord",
                                    .format = hl::VertexAttributeFormat::Vec2,
                                    .location = 1,
                                    .offset = offsetof(hl::Vertex22D, texCoord)
                                }
                            },
                            .stride = sizeof(hl::Vertex22D)
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
                        .enableBlending = true,
                        .pushConstantSize = sizeof(hl::TextPushConstantObject)
                    }
                }
            }
		};
	}

    // TODO: need to do dynamic amount of inputs
    RenderpassInfo RenderGraphHelpers::createCompositeRenderpassInfo(
        const std::vector<std::string>& inputs)
    {
        assert(inputs.size() == 2);

        return hl::RenderpassInfo
        {
            .name = "composite_pass",
            .useMultiSampling = false,
            .inputs = { inputs[0], inputs[1]},
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
                        .shaderVert = WellKnownResources::FullscreenSampleVertexShader_Name,
                        .shaderFrag = WellKnownResources::CompositeFragmentShader_Name,
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
                                        .resource = inputs[0]
                                    },
                                    hl::DescriptorBinding
                                    {
                                        .binding = 1,
                                        .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                        .stage = "FRAGMENT",
                                        .resource = inputs[1]
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
        };
    }
}