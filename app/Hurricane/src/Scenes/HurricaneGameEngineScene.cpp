#include "Scenes/HurricaneGameEngineScene.hpp"
#include "EntityPushConstantObject.hpp"
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/TextComponent.hpp>
#include <GLFW/glfw3.h>

namespace hur
{

	HurricaneGameEngineScene::HurricaneGameEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig
	) :
		EngineScene(engine),
		_engineConfig(engineConfig)
	{
		_camera = new hl::Camera2D();
		_engine.getEventBus().AddListener(this);
	}
	HurricaneGameEngineScene::~HurricaneGameEngineScene()
	{
		_engine.getEventBus().RemoveListener(this);

	}

	void HurricaneGameEngineScene::initialise(
		const std::string& cameraMatrixResourceId,
		hl::VulkanDevice& device,
		hl::VulkanSwapChain& swapChain,
		hl::VulkanCommandPool& graphicsCommandPool,
		hl::VulkanCommandPool& transferCommandPool,
		hl::ResourceManager& resourceManager,
		hl::MaterialSystem& materialSystem)
	{

        auto sceneRenderpassInfo = hl::RenderpassInfo
        {
            .name = "scene_pass",
            .useMultiSampling = false,
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
                        .name = "entity_pipeline",
                        .shaderVert = _engineConfig.RootPath + std::string("/data/shaders/entity.vert"),
                        .shaderFrag = _engineConfig.RootPath + std::string("/data/shaders/entity.frag"),
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
                                        .resource = cameraMatrixResourceId
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
                                    .offset = offsetof(hl::Vertex2, pos)
                                }
                            },
                            .stride = sizeof(hl::Vertex2)
                        },
                        .depthState =
                        {
                            .writeEnable = true,
                            .compareOp = VK_COMPARE_OP_LESS_OR_EQUAL
                        },
                        .rasterState =
                        {
                            .cullMode = VK_CULL_MODE_NONE
                        },
                        .enableBlending = false,
                        .pushConstantSize = sizeof(EntityPushConstantObject)
                    }
                }
            }
        };

        std::vector<hl::RenderpassInfo> renderpasses =
        {
            sceneRenderpassInfo,
            hl::RenderGraphHelpers::createTextRenderpassInfo(cameraMatrixResourceId),
            // TODO: ref vars for the outputs???
            hl::RenderGraphHelpers::createCompositeRenderpassInfo({ "scene_color", "text_color" })
        };

        hl::ResourceContext resourceContext
        {
            .device = &device,
            .pool = &transferCommandPool,
            .resourceManager = &resourceManager,
            .materialSystem = &materialSystem,
            .rootPath = _engineConfig.RootPath
        };

        // TODO: Move to base and generate texture programatically
        resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
            hl::MaterialSystem::FallbackTextureName,
            resourceContext);

        {
            auto entity = _scene.addEntity("game_state");
            entity->AddTag("TEXT");
            entity->AddComponent<hl::TransformComponent>();
            // TODO: Dont like having to pass text system here...
            entity->AddComponent<hl::TextComponent>()->setString(
                _engine.getTextSystem(),
                "Start",
                "roboto",
                64);
            entity->GetComponent<hl::TextComponent>()->setColour(glm::vec4(1.0f, 1.0f, 1.0f, 0.2f));
        }

        EngineScene::initialise(
            cameraMatrixResourceId,
            device,
            swapChain,
            graphicsCommandPool,
            transferCommandPool,
            resourceManager,
            materialSystem,
            renderpasses);

        registerPipelineDraw(
            "entity_pipeline",
            [&](hl::PipelineDrawData& pdd) -> void
            {

            });

        setGameState(GameState::INIT);

		handleWindowSizeChange(_engineConfig.Width, _engineConfig.Height);
	}

	void HurricaneGameEngineScene::update(uint32_t currentFrame, float delta)
	{

	}

	void HurricaneGameEngineScene::OnEvent(const hl::Event& event)
	{
        if (auto ke = dynamic_cast<const hl::KeyPressEvent*>(&event))
        {
            const auto code = ke->GetKeyCode();

            if (code == GLFW_KEY_ENTER)
            {
                std::cout << "ENTER PRESSED!" << std::endl;
            }
        }
		else if (auto wre = dynamic_cast<const hl::WindowResizeEvent*>(&event))
		{
			handleWindowSizeChange(wre->GetWidth(), wre->GetHeight());
		}
	}

	void HurricaneGameEngineScene::handleWindowSizeChange(int width, int height)
	{
        // TODO: Replace these with flags/enum for anchor/alignment, left, right, top, bottom, center -> 9 possibilities.
        const auto& centerTextAt = [&](const std::string& entityName, float yOffset) -> void
            {
                auto desiredCenter = glm::vec2(((float)width) / 2.0f, ((float)height) / 4.0f + yOffset);

                auto entity = _scene.getEntity(entityName);

                const auto& size = _engine
                    .getTextSystem()
                    .getTextSize(
                        entity->GetComponent<hl::TextComponent>()->getTextSystemId());

                desiredCenter.x += size.x - size.z / 2.0f;
                desiredCenter.y += size.y - size.w / 2.0f;

                entity->GetComponent<hl::TransformComponent>()->SetPosition(glm::vec3(desiredCenter, 0.0f));
            };


        centerTextAt("game_state", 0.0f);
	}

    void HurricaneGameEngineScene::setGameState(GameState state)
    {
        auto entity = _scene.getEntity("game_state");

        auto tc = entity->GetComponent<hl::TextComponent>();

        if (state == GameState::INIT)
        {
            tc->setString(
                _engine.getTextSystem(), 
                "INIT",
                "roboto",
                64);
        }
        else if (state == GameState::PLAYING)
        {
            tc->setString(
                _engine.getTextSystem(),
                "PLAYING",
                "roboto",
                64);
        }
        else if (state == GameState::GAME_OVER)
        {
            tc->setString(
                _engine.getTextSystem(),
                "GAME_OVER",
                "roboto",
                64);
        }
        else
        {
            tc->setString(
                _engine.getTextSystem(),
                "INVALID_STATE",
                "roboto",
                64);
        }

        state = state;
        
        handleWindowSizeChange(_engineConfig.Width, _engineConfig.Height);
    }
}