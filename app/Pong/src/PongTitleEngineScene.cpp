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
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>

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
            hl::RenderGraphHelpers::createTextRenderpassInfo(cameraMatrixResourceId)
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
        resourceManager.LoadAs<hl::SignedDistanceFieldFontResource, hl::FontResource>(
            "roboto", 
            resourceContext);
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
        const auto checkClick = _engine.getInputManager().isButtonReleased(GLFW_MOUSE_BUTTON_1);

        for (auto& e : _scene.getEntitiesWithComponents<hl::TransformComponent, hl::TextComponent>("TEXT"))
        {
            auto tc = e->GetComponent<hl::TransformComponent>();
            auto textComponent = e->GetComponent<hl::TextComponent>();
            const auto tcp = tc->GetPosition();

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
        // TODO: Replace these with flags/enum for anchor/alignment, left, right, top, bottom, center -> 9 possibilities.
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
            // end credits scene?
        }
        else if (name == "start")
        {
            _engine.setScene(new PongEngineScene(_engine, _engineConfig));
        }
        else if (name == "settings")
        {
            // TODO
        }
        else
        {
            std::cout << "Clicked on " << name << std::endl;
        }
    }
}