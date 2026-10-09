#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/System/Events/EventListener.hpp>
#include <Core/Session.hpp>

namespace drl
{

	class DrillerSplashEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		DrillerSplashEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig,
			Session& session);
		~DrillerSplashEngineScene();

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
		void handleWindowSizeChange(int width, int height);
		void setStatus(const std::string& text);

		const hl::EngineConfiguration& _engineConfig;
		Session& _session;
		int _loadStep{ 0 };
		bool _loadFailed{ false };
		bool _finished{ false };
	};

}
