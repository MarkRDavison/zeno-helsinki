#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Renderer/Resource/StorageBufferResource.hpp>
#include <helsinki/System/Resource/ResourceHandle.hpp>
#include <Core/Session.hpp>
#include <Views/JobView.hpp>
#include <Views/TerrainView.hpp>
#include <Views/WorkerView.hpp>

namespace drl
{

	class DrillerGameEngineScene : public hl::EngineScene
	{
	public:
		DrillerGameEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig,
			Session& session);
		void initialise(
			const std::string& cameraMatrixResourceId,
			hl::VulkanDevice& device,
			hl::VulkanSwapChain& swapChain,
			hl::VulkanCommandPool& graphicsCommandPool,
			hl::VulkanCommandPool& transferCommandPool,
			hl::ResourceManager& resourceManager) override;

		void update(uint32_t currentFrame, float delta) override;

	private:
		const hl::EngineConfiguration& _engineConfig;
		Session& _session;
		TerrainView _terrainView;
		JobView _jobView;
		WorkerView _workerView;
		hl::ResourceHandle<hl::StorageBufferResource> _spriteSheetSSBOResourceHandle;
	};

}
