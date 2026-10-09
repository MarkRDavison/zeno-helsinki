#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/TextPushConstantObject.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/MaterialPushConstantObject.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Renderer/RendererShaderRoot.hpp>
#include <helsinki/Renderer/Resource/TextSystem.hpp>
#include <helsinki/Renderer/Resource/ParticleSystem.hpp>
#include <helsinki/Renderer/Resource/MaterialSystem.hpp>
#include <string>

namespace hl
{

	RenderpassInfo RenderGraphHelpers::createTextRenderpassInfo(
        const std::string& cameraMatrixResourceId)
	{
		return RenderpassInfo
		{
            .name = "text_pass",
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
                        .shaderVert = std::string(RendererShaderRoot) + "/text.vert",
                        .shaderFrag = std::string(RendererShaderRoot) + "/text.frag",
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
                                        .resource = cameraMatrixResourceId,
                                        .count = MAX_CAMERAS,
                                        .updateFrequency = hl::DescriptorUpdateFrequency::Static
                                    },
                                    hl::DescriptorBinding
                                    {
                                        .binding = 1,
                                        .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                        .stage = "VERTEX&FRAGMENT",
                                        .resource = TextSystem::RasterAtlasName,
                                        .count = MAX_FONTS,
                                        .updateFrequency = hl::DescriptorUpdateFrequency::Static,
                                        .partiallyBound = true,
                                        .updateAfterBind = true
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
                        .shaderVert = std::string(RendererShaderRoot) + "/text.vert",
                        .shaderFrag = std::string(RendererShaderRoot) + "/sdf.frag",
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
                                        .resource = cameraMatrixResourceId,
                                        .count = MAX_CAMERAS,
                                        .updateFrequency = hl::DescriptorUpdateFrequency::Static
                                    },
                                    hl::DescriptorBinding
                                    {
                                        .binding = 1,
                                        .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                        .stage = "VERTEX&FRAGMENT",
                                        .resource = TextSystem::SdfAtlasName,
                                        .count = MAX_FONTS,
                                        .updateFrequency = hl::DescriptorUpdateFrequency::Static,
                                        .partiallyBound = true,
                                        .updateAfterBind = true
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
                        .shaderVert = std::string(RendererShaderRoot) + "/fullscreen_sample.vert",
                        .shaderFrag = std::string(RendererShaderRoot) + "/composite.frag",
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

	RenderpassInfo RenderGraphHelpers::createParticleSimPass()
	{
		return RenderpassInfo
		{
			.name = ParticleSystem::SimPassName,
			.bufferOutputs = { ParticleSystem::StorageBufferName },
			.pipelineGroups =
			{
				{
					PipelineInfo
					{
						.name = ParticleSystem::ComputePipelineName,
						.shaderComp = std::string(RendererShaderRoot) + "/particle_sim.comp",
						.bindPoint = PipelineBindPoint::Compute,
						.descriptorSets =
						{
							DescriptorSetInfo
							{
								.bindings =
								{
									DescriptorBinding
									{
										.binding = 0,
										.type = "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER",
										.stage = "COMPUTE",
										.resource = ParticleSystem::StorageBufferName
									},
									DescriptorBinding
									{
										.binding = 1,
										.type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
										.stage = "COMPUTE",
										.resource = ParticleSystem::EmitterUniformBufferName
									}
								}
							}
						}
					}
				}
			}
		};
	}

	PipelineInfo RenderGraphHelpers::particleDrawPipelineInfo(const std::string& cameraMatrixResourceId)
	{
		return PipelineInfo
		{
			.name = ParticleSystem::DrawPipelineName,
			.shaderVert = std::string(RendererShaderRoot) + "/particle.vert",
			.shaderFrag = std::string(RendererShaderRoot) + "/particle.frag",
			.descriptorSets =
			{
				DescriptorSetInfo
				{
					.bindings =
					{
						DescriptorBinding
						{
							.binding = 0,
							.type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
							.stage = "VERTEX",
							.resource = cameraMatrixResourceId,
							.count = MAX_CAMERAS
						},
						DescriptorBinding
						{
							.binding = 1,
							.type = "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER",
							.stage = "VERTEX",
							.resource = ParticleSystem::StorageBufferName
						},
						DescriptorBinding
						{
							.binding = 2,
							.type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
							.stage = "VERTEX",
							.resource = ParticleSystem::EmitterUniformBufferName
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
			.enableBlending = true,
			.additiveBlending = true,
			.topology = PrimitiveTopology::PointList
		};
	}

	PipelineInfo RenderGraphHelpers::particleQuadPipelineInfo(const std::string& cameraMatrixResourceId)
	{
		return PipelineInfo
		{
			.name = ParticleSystem::QuadPipelineName,
			.shaderVert = std::string(RendererShaderRoot) + "/particle_quad.vert",
			.shaderFrag = std::string(RendererShaderRoot) + "/particle_quad.frag",
			.descriptorSets =
			{
				DescriptorSetInfo
				{
					.bindings =
					{
						DescriptorBinding
						{
							.binding = 0,
							.type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
							.stage = "VERTEX",
							.resource = cameraMatrixResourceId,
							.count = MAX_CAMERAS
						},
						DescriptorBinding
						{
							.binding = 1,
							.type = "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER",
							.stage = "VERTEX",
							.resource = ParticleSystem::StorageBufferName
						},
						DescriptorBinding
						{
							.binding = 2,
							.type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
							.stage = "VERTEX",
							.resource = ParticleSystem::EmitterUniformBufferName
						},
						DescriptorBinding
						{
							.binding = 3,
							.type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
							.stage = "FRAGMENT",
							.resource = MaterialSystem::FallbackTextureName,
							.updateFrequency = DescriptorUpdateFrequency::Static
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
			.enableBlending = true,
			.additiveBlending = true,
			.topology = PrimitiveTopology::TriangleList
		};
	}

	PipelineInfo RenderGraphHelpers::shadowPipelineInfo(const std::string& shadowUboId)
	{
		return PipelineInfo
		{
			.name = ShadowPipelineName,
			.shaderVert = std::string(RendererShaderRoot) + "/shadow.vert",
			.shaderFrag = std::string(RendererShaderRoot) + "/shadow.frag",
			.descriptorSets =
			{
				DescriptorSetInfo
				{
					.bindings =
					{
						DescriptorBinding
						{
							.binding = 0,
							.type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
							.stage = "VERTEX",
							.resource = shadowUboId,
							.count = 1,
							.updateFrequency = DescriptorUpdateFrequency::PerFrame
						}
					}
				}
			},
			.vertexInputInfo = VertexInputInfo
			{
				.attributes =
				{
					{
						.name = "inPosition",
						.format = VertexAttributeFormat::Vec3,
						.location = 0,
						.offset = offsetof(Vertex, pos)
					},
					{
						.name = "inColor",
						.format = VertexAttributeFormat::Vec3,
						.location = 1,
						.offset = offsetof(Vertex, color)
					},
					{
						.name = "inTexCoord",
						.format = VertexAttributeFormat::Vec2,
						.location = 2,
						.offset = offsetof(Vertex, texCoord)
					},
					{
						.name = "inNormal",
						.format = VertexAttributeFormat::Vec3,
						.location = 3,
						.offset = offsetof(Vertex, normal)
					}
				},
				.stride = sizeof(Vertex)
			},
			.rasterState =
			{
				.cullMode = VK_CULL_MODE_FRONT_BIT
			},
			.pushConstantSize = sizeof(MaterialPushConstantObject)
		};
	}

	RenderpassInfo RenderGraphHelpers::createShadowMapPass(
		const std::string& depthName,
		uint32_t mapSize,
		const std::string& shadowUboId)
	{
		return RenderpassInfo
		{
			.name = ShadowPassName,
			.outputs =
			{
				ResourceInfo
				{
					.name = depthName,
					.type = ResourceType::Depth,
					.format = "VK_FORMAT_D32_SFLOAT"
				}
			},
			.pipelineGroups =
			{
				{ shadowPipelineInfo(shadowUboId) }
			},
			.extent = { .width = mapSize, .height = mapSize }
		};
	}

	VertexInputInfo RenderGraphHelpers::uiVertexInputInfo()
	{
		return VertexInputInfo
		{
			.attributes =
			{
				{
					.name = "inPosition",
					.format = VertexAttributeFormat::Vec2,
					.location = 0,
					.offset = offsetof(VertexUi2, pos)
				},
				{
					.name = "inColor",
					.format = VertexAttributeFormat::Vec4,
					.location = 1,
					.offset = offsetof(VertexUi2, color)
				},
				{
					.name = "inTexCoord",
					.format = VertexAttributeFormat::Vec2,
					.location = 2,
					.offset = offsetof(VertexUi2, texCoord)
				},
				{
					.name = "inTexIndex",
					.format = VertexAttributeFormat::Float,
					.location = 3,
					.offset = offsetof(VertexUi2, texIndex)
				},
				{
					.name = "inSdf",
					.format = VertexAttributeFormat::Float,
					.location = 4,
					.offset = offsetof(VertexUi2, sdf)
				}
			},
			.stride = sizeof(VertexUi2)
		};
	}
}