#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/System/Resource/ResourceHandle.hpp>
#include <helsinki/Renderer/Resource/UniformBufferResource.hpp>
#include <helsinki/System/Events/EventListener.hpp>
#include <helsinki/System/glm.hpp>

namespace hl
{
	class Camera;
}

namespace sk
{

	class SkeletonEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		SkeletonEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig,
			bool enableGpuParticles);
		~SkeletonEngineScene();
		void initialise(
			const std::string& cameraMatrixResourceId,
			hl::VulkanDevice& device,
			hl::VulkanSwapChain& swapChain,
			hl::VulkanCommandPool& graphicsCommandPool,
			hl::VulkanCommandPool& transferCommandPool,
			hl::ResourceManager& resourceManager) override;

		void update(uint32_t currentFrame, float delta) override;
		void updateGpuResources(uint32_t currentFrame) override;
		void OnEvent(const hl::Event& event) override;
	private:
		std::vector<hl::RenderpassInfo> buildRenderpasses() const;
		void spawnScene(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void updateOrbit();
		hl::Camera* defaultCamera() const;

		const hl::EngineConfiguration& _engineConfig;
		bool _enableGpuParticles = false;
		hl::ResourceHandle<hl::UniformBufferResource> _sunUbo;
		hl::ResourceHandle<hl::UniformBufferResource> _pointLightsUbo;
		bool _specHeldOff = false;
		float _cameraDistance = 0.0f;
		bool _orbitDragging = false;
		glm::vec2 _orbitStartMouse{ 0.0f };
		float _orbitStartYaw = 0.0f;
		float _orbitStartPitch = 0.0f;
	};

}
