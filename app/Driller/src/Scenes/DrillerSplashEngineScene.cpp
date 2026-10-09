#include <Scenes/DrillerSplashEngineScene.hpp>
#include <Scenes/DrillerTitleEngineScene.hpp>
#include <Scenes/DrillerGameEngineScene.hpp>
#include <Core/LoadError.hpp>
#include <helsinki/Scripting/LuaError.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Renderer/Resource/SignedDistanceFieldFontResource.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/TextComponent.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <helsinki/Renderer/Resource/TextSystem.hpp>
#include <helsinki/Renderer/Resource/MaterialSystem.hpp>
#include <format>

namespace drl
{

	DrillerSplashEngineScene::DrillerSplashEngineScene(
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

	DrillerSplashEngineScene::~DrillerSplashEngineScene()
	{
		_engine.getEventBus().RemoveListener(this);
	}

	void DrillerSplashEngineScene::initialise(
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
			auto entity = _scene.addEntity("status");
			entity->AddTag("TEXT");
			entity->AddComponent<hl::TransformComponent>();
			entity->AddComponent<hl::TextComponent>()->setString(
				_engine.getTextSystem(),
				"Loading 0/3",
				"roboto",
				48);
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

	void DrillerSplashEngineScene::update(uint32_t /*currentFrame*/, float /*delta*/)
	{
		if (_finished || _loadFailed)
		{
			return;
		}

		++_loadStep;
		setStatus(std::format("Loading {}/3", _loadStep));

		if (_loadStep < 3)
		{
			return;
		}

		try
		{
			_session.loadAndValidate();
		}
		catch (const LoadError& error)
		{
			_loadFailed = true;
			setStatus(error.what());
			return;
		}
		catch (const hl::scripting::LuaError& error)
		{
			_loadFailed = true;
			setStatus(error.what());
			return;
		}

		_finished = true;
		if (_session.skipToGameplay())
		{
			_engine.setScene(new DrillerGameEngineScene(_engine, _engineConfig, _session));
		}
		else
		{
			_engine.setScene(new DrillerTitleEngineScene(_engine, _engineConfig, _session));
		}
	}

	void DrillerSplashEngineScene::OnEvent(const hl::Event& event)
	{
		if (auto wre = dynamic_cast<const hl::WindowResizeEvent*>(&event))
		{
			handleWindowSizeChange(wre->GetWidth(), wre->GetHeight());
		}
	}

	void DrillerSplashEngineScene::handleWindowSizeChange(int width, int height)
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
		centerTextAt("status", 1.0f * height / 6.0f);
	}

	void DrillerSplashEngineScene::setStatus(const std::string& text)
	{
		auto entity = _scene.getEntity("status");
		entity->GetComponent<hl::TextComponent>()->setString(
			_engine.getTextSystem(),
			text,
			"roboto",
			48);
		handleWindowSizeChange(_engineConfig.Width, _engineConfig.Height);
	}

}
