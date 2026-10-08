#pragma once

#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Physics/Physics.hpp>
#include <helsinki/System/Events/EventListener.hpp>
#include <helsinki/System/glm.hpp>
#include <memory>
#include <vector>

namespace phys
{
	class Platformer2DEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		Platformer2DEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig,
			hl::physics::Context& physicsContext);
		~Platformer2DEngineScene();

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
		void applyInput();
		void syncTransformsFromWorld();
		bool ladderAabbOverlapsCharacter() const;
		bool isCrate(hl::physics::BodyId id) const;

		const hl::EngineConfiguration& _engineConfig;
		hl::physics::Context& _physicsContext;
		std::unique_ptr<hl::physics::World> _world;

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
	};
}
