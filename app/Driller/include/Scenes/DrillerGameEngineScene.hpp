#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Engine/Ui/UiBatch.hpp>
#include <helsinki/Renderer/Resource/StorageBufferResource.hpp>
#include <helsinki/System/Events/EventListener.hpp>
#include <helsinki/System/Resource/ResourceHandle.hpp>
#include <Core/Session.hpp>
#include <Views/Hud.hpp>
#include <Views/BuildingGhostView.hpp>
#include <Views/BuildingView.hpp>
#include <Views/GameCamera.hpp>
#include <Views/JobView.hpp>
#include <Views/ShuttleView.hpp>
#include <Views/TerrainView.hpp>
#include <Views/WorkerView.hpp>

namespace drl
{

	class DrillerGameEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		DrillerGameEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig,
			Session& session);
		~DrillerGameEngineScene();

		void initialise(
			const std::string& cameraMatrixResourceId,
			hl::VulkanDevice& device,
			hl::VulkanSwapChain& swapChain,
			hl::VulkanCommandPool& graphicsCommandPool,
			hl::VulkanCommandPool& transferCommandPool,
			hl::ResourceManager& resourceManager) override;

		void update(uint32_t currentFrame, float delta) override;
		void updateGpuResources(uint32_t currentFrame) override;
		void additionalCleanup() override;
		void OnEvent(const hl::Event& event) override;

	private:
		glm::vec2 framebufferMouse() const;
		void bindGameCameraToViews();

		const hl::EngineConfiguration& _engineConfig;
		Session& _session;
		TerrainView _terrainView;
		BuildingView _buildingView;
		BuildingGhostView _buildingGhostView;
		JobView _jobView;
		WorkerView _workerView;
		ShuttleView _shuttleView;
		GameCamera* _gameCamera{ nullptr };
		glm::ivec2 _hoveredTile{ 0, -1 };
		glm::vec2 _lastPanMouse{ 0.0f, 0.0f };
		bool _panning{ false };
		bool _blockedSeekShown{ false };
		Hud _hud;
		hl::UiBatch _uiBatch;
		hl::ResourceHandle<hl::StorageBufferResource> _spriteSheetSSBOResourceHandle;
	};

}
