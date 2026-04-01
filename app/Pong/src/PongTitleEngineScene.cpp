#include "PongTitleEngineScene.hpp"
#include "PongEngineScene.hpp"
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/TextPushConstantObject.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Renderer/Resource/SignedDistanceFieldFontResource.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

namespace pong
{

	PongTitleEngineScene::PongTitleEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig
	) :
		EngineScene(engine),
		_engineConfig(engineConfig)
	{
		_camera = new hl::Camera2D();
		_engine.getEventBus().AddListener(this);
	}

	PongTitleEngineScene::~PongTitleEngineScene()
	{
		_engine.getEventBus().RemoveListener(this);
	}

	void PongTitleEngineScene::initialise(
		const std::string& cameraMatrixResourceId,
		hl::VulkanDevice& device,
		hl::VulkanSwapChain& swapChain,
		hl::VulkanCommandPool& graphicsCommandPool,
		hl::VulkanCommandPool& transferCommandPool,
		hl::ResourceManager& resourceManager,
		hl::MaterialSystem& materialSystem)
	{
		std::vector<hl::RenderpassInfo> renderpasses
		{
			hl::RenderpassInfo
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
                            .shaderVert = _engineConfig.RootPath + std::string("/data/shaders/text.vert"),
                            .shaderFrag = _engineConfig.RootPath + std::string("/data/shaders/text.frag"),
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
                                            .resource = "camera_matrix_ubo"
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
                            .enableBlending = true, // keep blending for UI elements
                            .pushConstantSize = sizeof(hl::TextPushConstantObject)
                        },
                        hl::PipelineInfo
                        {
                            .name = "sdf_text_pipeline",
                            .shaderVert = _engineConfig.RootPath + std::string("/data/shaders/text.vert"),
                            .shaderFrag = _engineConfig.RootPath + std::string("/data/shaders/sdf.frag"),
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
                                            .resource = "camera_matrix_ubo"
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
                            .enableBlending = true, // keep blending for UI elements
                            .pushConstantSize = sizeof(hl::TextPushConstantObject)
                        }
                    }
                }
			}
		};

		hl::ResourceContext resourceContext
		{
			.device = &device,
			.pool = &transferCommandPool,
			.resourceManager = &resourceManager,
			.materialSystem = &materialSystem,
			.rootPath = _engineConfig.RootPath
		};

        resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
            "white",
            resourceContext);

        resourceManager.LoadAs<hl::SignedDistanceFieldFontResource, hl::FontResource>("roboto", resourceContext);
        resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
            "roboto",
            resourceContext);

		EngineScene::initialise(
			cameraMatrixResourceId,
			device,
			swapChain,
			graphicsCommandPool,
			transferCommandPool,
			resourceManager,
			materialSystem,
			renderpasses);
	}

	void PongTitleEngineScene::update(uint32_t currentFrame, float delta)
	{

	}

	void PongTitleEngineScene::OnEvent(const hl::Event& event)
	{
        if (auto ke = dynamic_cast<const hl::KeyPressEvent*>(&event))
        {
            const auto code = ke->GetKeyCode();

            if (code == GLFW_KEY_ENTER)
            {
                std::cout << "ENTER PRESSED!" << std::endl;
                _engine.setScene(new PongEngineScene(_engine, _engineConfig));
            }
        }
	}
}