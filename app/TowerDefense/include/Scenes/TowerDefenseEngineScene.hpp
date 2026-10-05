#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/System/Resource/ResourceHandle.hpp>
#include <helsinki/Renderer/Resource/UniformBufferResource.hpp>
#include <helsinki/Engine/ECS/Entity.hpp>

namespace tower
{

	class TowerDefenseEngineScene : public hl::EngineScene
	{
	public:
		TowerDefenseEngineScene(hl::Engine& engine, const hl::EngineConfiguration& engineConfig);
		void initialise(
			const std::string& cameraMatrixResourceId,
			hl::VulkanDevice& device,
			hl::VulkanSwapChain& swapChain,
			hl::VulkanCommandPool& graphicsCommandPool,
			hl::VulkanCommandPool& transferCommandPool,
			hl::ResourceManager& resourceManager) override;

		void update(uint32_t currentFrame, float delta) override;
		void updateGpuResources(uint32_t currentFrame) override;
	private:
		std::vector<hl::RenderpassInfo> buildRenderpasses() const;
		void spawnScene(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void spawnBoard(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void spawnTurret(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void spawnCreep(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void spawnMarker(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void tryMoveMarkerToClick();

		const hl::EngineConfiguration& _engineConfig;
		hl::ResourceHandle<hl::UniformBufferResource> _sunUbo;
		hl::Entity* _marker = nullptr;
	};

}
