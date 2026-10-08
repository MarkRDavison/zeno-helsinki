#pragma once

#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Physics/Physics.hpp>
#include <helsinki/Renderer/Resource/UniformBufferResource.hpp>
#include <helsinki/System/Events/EventListener.hpp>
#include <helsinki/System/Infrastructure/Camera.hpp>
#include <helsinki/System/Resource/ResourceHandle.hpp>
#include <helsinki/System/glm.hpp>
#include <memory>
#include <vector>

namespace phys
{
	class Platformer3DEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		Platformer3DEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig,
			hl::physics::Context& physicsContext);
		~Platformer3DEngineScene();

		void initialise(
			const std::string& cameraMatrixResourceId,
			hl::VulkanDevice& device,
			hl::VulkanSwapChain& swapChain,
			hl::VulkanCommandPool& graphicsCommandPool,
			hl::VulkanCommandPool& transferCommandPool,
			hl::ResourceManager& resourceManager) override;

		void update(uint32_t currentFrame, float delta) override;
		void additionalCleanup() override;
		void OnEvent(const hl::Event& event) override;

	private:
		void handleWindowSizeChange(int width, int height);
		void buildCourse();
		hl::Entity* addVisual(
			const std::string& name,
			const glm::vec3& visualHalfExtents,
			const glm::vec4& color,
			bool isCharacter);
		void applyInput(float delta);
		void updateOrbit(float delta);
		void syncTransformsFromWorld();
		bool ladderAabbOverlapsCharacter() const;
		bool isCrate(hl::physics::BodyId id) const;

		const hl::EngineConfiguration& _engineConfig;
		hl::physics::Context& _physicsContext;
		std::unique_ptr<hl::physics::World> _world;
		hl::Camera* _worldCamera = nullptr;
		hl::ResourceHandle<hl::UniformBufferResource> _sunUbo;

		hl::physics::CharacterId _player;
		hl::physics::BodyId _ladder;
		std::vector<hl::physics::BodyId> _crates;
		glm::vec3 _ladderPosition{0.f};
		glm::vec3 _ladderHalfExtents{0.f};

		int _viewportWidth = 0;
		int _viewportHeight = 0;
		bool _jumpQueued = false;
		int _airJumpsLeft = 1;
		bool _ladderContactThisStep = false;
		bool _ladderOverlap = false;

		float _orbitYaw = -90.f;
		float _orbitPitch = -20.f;
		float _orbitDistance = 10.f;
		bool _orbitDragging = false;
		glm::vec2 _lastMouse{0.f};
	};
}
