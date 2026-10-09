#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/System/Events/EventListener.hpp>
#include <Core/Session.hpp>

namespace drl
{

	class DrillerTitleEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		DrillerTitleEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig,
			Session& session);
		~DrillerTitleEngineScene();

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
		void goGame();

		const hl::EngineConfiguration& _engineConfig;
		Session& _session;
	};

}
