#pragma once

#include <Views/GalaxyCamera.hpp>
#include <Views/GalaxyView.hpp>
#include <Views/TimeChrome.hpp>
#include <World/SimClock.hpp>
#include <World/WorldData.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Imgui/Context.hpp>
#include <helsinki/System/Events/EventListener.hpp>
#include <helsinki/System/glm.hpp>

namespace slr
{

	class GameEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		GameEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig,
			hl::imgui::Context& imgui);
		~GameEngineScene();

		void initialise(
			const std::string& cameraMatrixResourceId,
			hl::VulkanDevice& device,
			hl::VulkanSwapChain& swapChain,
			hl::VulkanCommandPool& graphicsCommandPool,
			hl::VulkanCommandPool& transferCommandPool,
			hl::ResourceManager& resourceManager) override;

		void update(uint32_t currentFrame, float delta) override;
		void OnEvent(const hl::Event& event) override;

	private:
		const hl::EngineConfiguration& _engineConfig;
		hl::imgui::Context& _imgui;
		WorldData _world;
		SimClock _clock;
		TimeChrome _timeChrome;
		GalaxyView _galaxy;
		GalaxyCamera* _galaxyCamera{ nullptr };
		bool _panning{ false };
		glm::vec2 _lastMouse{ 0.0f };
	};

}
