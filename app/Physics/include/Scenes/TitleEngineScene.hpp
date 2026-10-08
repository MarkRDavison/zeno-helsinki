#pragma once

#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Physics/Context.hpp>
#include <helsinki/System/Events/EventListener.hpp>

namespace phys
{
	class TitleEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		TitleEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig,
			hl::physics::Context& physicsContext);
		~TitleEngineScene();

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
		void handleTextClicked(const std::string& name);

		const hl::EngineConfiguration& _engineConfig;
		hl::physics::Context& _physicsContext;
	};
}
