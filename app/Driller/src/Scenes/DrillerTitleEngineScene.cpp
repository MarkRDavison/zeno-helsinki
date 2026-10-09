#include <Scenes/DrillerTitleEngineScene.hpp>
#include <Scenes/DrillerGameEngineScene.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Renderer/Resource/SignedDistanceFieldFontResource.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/TextComponent.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <helsinki/Renderer/Resource/TextSystem.hpp>
#include <helsinki/Renderer/Resource/MaterialSystem.hpp>
#include <GLFW/glfw3.h>

namespace drl
{

	DrillerTitleEngineScene::DrillerTitleEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig,
		Session& session
	) :
		EngineScene(engine),
		_engineConfig(engineConfig),
		_session(session)
	{
		_cameras.insert({ "Default", new hl::Camera2D() });
		_engine.getEventBus().AddListener(this);
	}

	DrillerTitleEngineScene::~DrillerTitleEngineScene()
	{
		_engine.getEventBus().RemoveListener(this);
	}

	void DrillerTitleEngineScene::initialise(
		const std::string& cameraMatrixResourceId,
		hl::VulkanDevice& device,
		hl::VulkanSwapChain& swapChain,
		hl::VulkanCommandPool& graphicsCommandPool,
		hl::VulkanCommandPool& transferCommandPool,
		hl::ResourceManager& resourceManager)
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
			.materialSystem = &_engine.getMaterialSystem(),
			.rootPath = _engineConfig.RootPath
		};

		resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
			hl::MaterialSystem::FallbackTextureName,
			resourceContext);
		resourceManager.LoadAs<hl::SignedDistanceFieldFontResource, hl::FontResource>(
			"roboto",
			resourceContext);
		resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
			"roboto",
			resourceContext);

		resourceManager.LoadLogical(
			hl::ResourceDefinition
			{
				.name = hl::TextSystem::RasterAtlasName,
				.type = "logical",
				.resources = { { .name = hl::MaterialSystem::FallbackTextureName, .type = "texture" } }
			},
			[&](const hl::ResourceDefinition::Child& child)
			{
				return resourceManager.HasResource<hl::ImageSamplerResource>(child.name);
			});
		resourceManager.LoadLogical(
			hl::ResourceDefinition
			{
				.name = hl::TextSystem::SdfAtlasName,
				.type = "logical",
				.resources = { { .name = "roboto", .type = "texture" } }
			},
			[&](const hl::ResourceDefinition::Child& child)
			{
				return resourceManager.HasResource<hl::ImageSamplerResource>(child.name);
			});

		{
			auto entity = _scene.addEntity("title");
			entity->AddTag("TEXT");
			entity->AddComponent<hl::TransformComponent>();
			entity->AddComponent<hl::TextComponent>()->setString(
				_engine.getTextSystem(),
				"DRILLER",
				"roboto",
				128);
			entity->GetComponent<hl::TextComponent>()->setColour(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
		}
		{
			auto entity = _scene.addEntity("play");
			entity->AddTag("TEXT");
			entity->AddComponent<hl::TransformComponent>();
			entity->AddComponent<hl::TextComponent>()->setString(
				_engine.getTextSystem(),
				"Play",
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
			renderpasses);
	}

	void DrillerTitleEngineScene::update(uint32_t /*currentFrame*/, float /*delta*/)
	{
		const auto mouse = _engine.getInputManager().getMousePosition();
		const auto checkClick = _engine.getInputManager().isButtonReleased(GLFW_MOUSE_BUTTON_1);

		auto play = _scene.getEntity("play");
		auto tc = play->GetComponent<hl::TransformComponent>();
		auto textComponent = play->GetComponent<hl::TextComponent>();
		const auto tcp = tc->GetPosition();
		const auto& size = _engine.getTextSystem().getTextSize(textComponent->getTextSystemId());

		if ((tcp.x + size.x <= mouse.x) && (mouse.x <= tcp.x + size.z) &&
			(tcp.y + size.y <= mouse.y) && (mouse.y <= tcp.y + size.w))
		{
			textComponent->setColour(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
			if (checkClick)
			{
				goGame();
			}
		}
		else
		{
			textComponent->setColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		}
	}

	void DrillerTitleEngineScene::OnEvent(const hl::Event& event)
	{
		if (auto ke = dynamic_cast<const hl::KeyPressEvent*>(&event))
		{
			if (ke->GetKeyCode() == GLFW_KEY_ENTER)
			{
				goGame();
			}
		}
		else if (auto wre = dynamic_cast<const hl::WindowResizeEvent*>(&event))
		{
			handleWindowSizeChange(wre->GetWidth(), wre->GetHeight());
		}
	}

	void DrillerTitleEngineScene::handleWindowSizeChange(int width, int height)
	{
		const auto centerTextAt = [&](const std::string& entityName, float yOffset) -> void
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

		centerTextAt("title", 0.0f);
		centerTextAt("play", 1.0f * height / 6.0f);
	}

	void DrillerTitleEngineScene::goGame()
	{
		_engine.setScene(new DrillerGameEngineScene(_engine, _engineConfig, _session));
	}

}
