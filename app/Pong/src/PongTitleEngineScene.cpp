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
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/TextComponent.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>

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

        // TODO: Helper methods to create these standard renderpasses???
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

        {
            auto entity = _scene.addEntity("title");
            entity->AddTag("TEXT");
            entity->AddComponent<hl::TransformComponent>();
            // TODO: Dont like having to pass text system here...
            entity->AddComponent<hl::TextComponent>()->setString(
                _engine.getTextSystem(),
                "PONG",
                "roboto",
                128);
            entity->GetComponent<hl::TextComponent>()->setColour(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
        }
        {
            auto entity = _scene.addEntity("start");
            entity->AddTag("TEXT");
            entity->AddComponent<hl::TransformComponent>();
            // TODO: Dont like having to pass text system here...
            entity->AddComponent<hl::TextComponent>()->setString(
                _engine.getTextSystem(),
                "Start",
                "roboto",
                64);
        }
        {
            auto entity = _scene.addEntity("quit");
            entity->AddTag("TEXT");
            entity->AddComponent<hl::TransformComponent>();
            // TODO: Dont like having to pass text system here...
            entity->AddComponent<hl::TextComponent>()->setString(
                _engine.getTextSystem(),
                "Quit",
                "roboto",
                64);
        }
        {
            auto entity = _scene.addEntity("topleft");
            entity->AddTag("TEXT");
            entity->AddComponent<hl::TransformComponent>();
            // TODO: Dont like having to pass text system here...
            entity->AddComponent<hl::TextComponent>()->setString(
                _engine.getTextSystem(),
                "TL",
                "roboto",
                64);
        }
        {
            auto entity = _scene.addEntity("bottomright");
            entity->AddTag("TEXT");
            entity->AddComponent<hl::TransformComponent>();
            // TODO: Dont like having to pass text system here...
            entity->AddComponent<hl::TextComponent>()->setString(
                _engine.getTextSystem(),
                "BR",
                "roboto",
                64);
        }

        handleWindowSizeChange(_engineConfig.Width, _engineConfig.Height);

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
        const auto mouse = _engine.getInputManager().getMousePosition();

        auto wasPressedLastFrame = _wasPressed;

        _wasPressed = _engine.getInputManager().isButtonDown(GLFW_MOUSE_BUTTON_1);

        const auto checkClick = wasPressedLastFrame && !_wasPressed;

        for (auto& e : _scene.getEntities())
        {
            if (!e->HasTag("TEXT") ||
                !e->HasComponents<hl::TransformComponent, hl::TextComponent>())
            {
                continue;
            }

            auto tc = e->GetComponent<hl::TransformComponent>();
            auto textComponent = e->GetComponent<hl::TextComponent>();
            auto tcp = tc->GetPosition();

            const auto& size = _engine.getTextSystem().getTextSize(textComponent->getTextSystemId());

            if ((tcp.x + size.x <= mouse.x) && (mouse.x <= tcp.x + size.z) &&
                (tcp.y + size.y <= mouse.y) && (mouse.y <= tcp.y + size.w))
            {
                textComponent->setColour(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

                if (checkClick)
                {
                    handleTextClicked(e->getName());
                }
            }
            else
            {
                textComponent->setColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
        }
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
        else if (auto wre = dynamic_cast<const hl::WindowResizeEvent*>(&event))
        {
            handleWindowSizeChange(wre->GetWidth(), wre->GetHeight());
        }
	}

    void PongTitleEngineScene::handleWindowSizeChange(int width, int height)
    {
        const auto& centerTextAt = [&](const std::string & entityName, float yOffset) -> void
        {
            auto desiredCenter = glm::vec2(((float)width) / 2.0f, ((float)height) / 3.0f + yOffset);

            auto entity = _scene.getEntity(entityName);

            const auto& size = _engine
                .getTextSystem()
                .getTextSize(
                    entity->GetComponent<hl::TextComponent>()->getTextSystemId());

            desiredCenter.x += size.x - size.z / 2.0f;
            desiredCenter.y += size.y - size.w / 2.0f;

            entity->GetComponent<hl::TransformComponent>()->SetPosition(glm::vec3(desiredCenter, 0.0f));
        };

        // TODO: Replace this with flags/enum for anchor/alignment, left, right, top, bottom, center -> 9 possibilities.
        const auto& alignTextTopLeft = [&](const std::string & entityName, float x, float y) -> void
        {
            auto desiredPosition = glm::vec2(x, y);

            auto entity = _scene.getEntity(entityName);

            const auto& size = _engine
                .getTextSystem()
                .getTextSize(
                    entity->GetComponent<hl::TextComponent>()->getTextSystemId());

            entity->GetComponent<hl::TransformComponent>()->SetPosition(glm::vec3(desiredPosition, 0.0f));
        };

        const auto& alignTextBottomRight = [&](const std::string & entityName, float x, float y) -> void
        {
            auto desiredPosition = glm::vec2(width, height);

            auto entity = _scene.getEntity(entityName);

            const auto& size = _engine
                .getTextSystem()
                .getTextSize(
                    entity->GetComponent<hl::TextComponent>()->getTextSystemId());

            desiredPosition.x += size.x - size.z;
            desiredPosition.y += size.y - size.w;

            entity->GetComponent<hl::TransformComponent>()->SetPosition(glm::vec3(desiredPosition, 0.0f));
        };

        centerTextAt("title", 0.0f);
        centerTextAt("start", 1.0f * height / 6.0f);
        centerTextAt("quit", 2.0f * height / 6.0f);
        alignTextTopLeft("topleft", 0.0f, 0.0f);
        alignTextBottomRight("bottomright", 0.0f, 0.0f);
    }

    void PongTitleEngineScene::handleTextClicked(const std::string& name)
    {
        if (name == "quit")
        {
            _engine.stop();
        }
        else if (name == "start")
        {
            _engine.setScene(new PongEngineScene(_engine, _engineConfig));
        }
        else
        {
            std::cout << "Clicked on " << name << std::endl;
        }
    }
}