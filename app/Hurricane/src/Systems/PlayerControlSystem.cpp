#include <Systems/PlayerControlSystem.hpp>
#include <Components/EntityComponent.hpp>
#include <Components/PlayerLoadoutComponent.hpp>
#include <Events/BombEvent.hpp>
#include <Events/ShootEvent.hpp>
#include <HurricaneConstants.hpp>
#include <WeaponCatalog.hpp>
#include <helsinki/Engine/ECS/Components/SpriteComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>

namespace hur
{

	PlayerControlSystem::PlayerControlSystem(
		hl::InputManager& inputManager,
		hl::EventBus& eventBus,
		hl::Scene& scene,
		const ResourceService& resourceService
	) :
		_inputManager(inputManager),
		_eventBus(eventBus),
		_scene(scene),
		_resourceService(resourceService)
	{

	}

	void PlayerControlSystem::spawnPlayerEngine()
	{
		auto* entity = _scene.addEntity(PlayerEngineEntityName);
		entity->AddTag("SPRITE");
		entity->AddTag("PLAYER_FX");

		auto* ec = entity->AddComponent<EntityComponent>();
		ec->SpriteName = PlayerEngineSprite;
		ec->Size = _resourceService.getSize(PlayerEngineSprite);

		entity->AddComponent<hl::SpriteComponent>()->setFrameDataIndex(
			static_cast<int>(_resourceService.getIndex(PlayerEngineSprite)));
		entity->AddComponent<hl::TransformComponent>();
	}

	void PlayerControlSystem::syncPlayerEngine(const hl::Entity& player)
	{
		auto* engine = _scene.getEntity(PlayerEngineEntityName);
		if (engine == nullptr || _scene.isPendingRemoval(engine->Id))
		{
			return;
		}

		auto* playerTransform = player.GetComponent<hl::TransformComponent>();
		auto* playerEntity = player.GetComponent<EntityComponent>();
		auto* engineTransform = engine->GetComponent<hl::TransformComponent>();
		auto* engineEntity = engine->GetComponent<EntityComponent>();
		if (playerTransform == nullptr
			|| playerEntity == nullptr
			|| engineTransform == nullptr
			|| engineEntity == nullptr)
		{
			return;
		}

		const auto playerPos = playerTransform->GetPosition();
		engineTransform->SetPosition({
			playerPos.x,
			playerPos.y + playerEntity->Size.y * 0.5f + engineEntity->Size.y * 0.5f - PlayerEngineOverlap,
			playerPos.z
		});
	}

	void PlayerControlSystem::destroyPlayerEngine()
	{
		if (auto* engine = _scene.getEntity(PlayerEngineEntityName))
		{
			if (!_scene.isPendingRemoval(engine->Id))
			{
				_scene.removeEntity(engine->Id);
			}
		}
	}

	void PlayerControlSystem::spawnPlayerShield()
	{
		auto* entity = _scene.addEntity(PlayerShieldEntityName);
		entity->AddTag("SPRITE");
		entity->AddTag("PLAYER_FX");
		entity->AddComponent<EntityComponent>();
		entity->AddComponent<hl::SpriteComponent>();
		entity->AddComponent<hl::TransformComponent>();
	}

	void PlayerControlSystem::syncPlayerShield(const hl::Entity& player, int shieldLayers)
	{
		auto* shield = _scene.getEntity(PlayerShieldEntityName);
		if (shield == nullptr || _scene.isPendingRemoval(shield->Id))
		{
			return;
		}

		if (shieldLayers <= 0 || shieldLayers > PlayerShieldMaxLayers)
		{
			return;
		}

		auto* playerTransform = player.GetComponent<hl::TransformComponent>();
		auto* shieldTransform = shield->GetComponent<hl::TransformComponent>();
		auto* shieldEntity = shield->GetComponent<EntityComponent>();
		auto* shieldSprite = shield->GetComponent<hl::SpriteComponent>();
		if (playerTransform == nullptr
			|| shieldTransform == nullptr
			|| shieldEntity == nullptr
			|| shieldSprite == nullptr)
		{
			return;
		}

		const char* spriteName = PlayerShieldSpriteNames[shieldLayers - 1];
		shieldEntity->SpriteName = spriteName;
		shieldEntity->Size = _resourceService.getSize(spriteName);
		shieldSprite->setFrameDataIndex(static_cast<int>(_resourceService.getIndex(spriteName)));
		shieldTransform->SetPosition(playerTransform->GetPosition());
	}

	void PlayerControlSystem::destroyPlayerShield()
	{
		if (auto* shield = _scene.getEntity(PlayerShieldEntityName))
		{
			if (!_scene.isPendingRemoval(shield->Id))
			{
				_scene.removeEntity(shield->Id);
			}
		}
	}

	void PlayerControlSystem::update(float delta)
	{
		auto player = _scene.getEntity("Player");

		if (player == nullptr)
		{
			destroyPlayerEngine();
			destroyPlayerShield();
			return;
		}

		auto tc = player->GetComponent<hl::TransformComponent>();
		auto sc = player->GetComponent<EntityComponent>();
		auto* loadout = player->GetComponent<PlayerLoadoutComponent>();

		if (loadout != nullptr)
		{
			loadout->speedBoostRemaining = std::max(
				0.0f,
				loadout->speedBoostRemaining - delta);
		}

		const bool boostActive = loadout != nullptr && loadout->speedBoostRemaining > 0.0f;
		const float SPEED = 256.0f * (boostActive ? SpeedBoostMultiplier : 1.0f);

		glm::vec2 movement{};

		if (auto* weapon = player->GetComponent<WeaponComponent>())
		{
			weapon->fireCooldownRemaining = std::max(
				0.0f,
				weapon->fireCooldownRemaining - delta);

			if (_inputManager.isKeyDown(GLFW_KEY_SPACE)
				&& weapon->fireCooldownRemaining <= 0.0f)
			{
				_eventBus.PublishEvent(ShootEvent(player->Id));
				weapon->fireCooldownRemaining = weapon->secondsPerShot;
			}
		}

		if (_inputManager.isKeyReleased(GLFW_KEY_LEFT_SHIFT))
		{
			if (loadout != nullptr && loadout->bombs > 0)
			{
				--loadout->bombs;
				_eventBus.PublishEvent(BombEvent(player->Id));
			}
		}

		if (_inputManager.isKeyDown(GLFW_KEY_A))
		{
			movement.x -= 1.0f;
		}

		if (_inputManager.isKeyDown(GLFW_KEY_D))
		{
			movement.x += 1.0f;
		}
		if (_inputManager.isKeyDown(GLFW_KEY_W))
		{
			movement.y -= 1.0f;
		}

		if (_inputManager.isKeyDown(GLFW_KEY_S))
		{
			movement.y += 1.0f;
		}

		if (movement.x != 0.0f || movement.y != 0.0f)
		{

			const auto pos = tc->GetPosition();

			movement = glm::normalize(movement) * delta * SPEED;

			auto newPosition = glm::vec2(pos.x + movement.x, pos.y + movement.y);

			if (newPosition.x - sc->Size.x / 2.0f < 0.0f)
			{
				newPosition.x = sc->Size.x / 2.0f;
			}
			else if (newPosition.x + sc->Size.x / 2.0f > HurricaneConstants::Width)
			{
				newPosition.x = HurricaneConstants::Width - sc->Size.x / 2.0f;
			}

			if (newPosition.y - sc->Size.y / 2.0f < 0.0f)
			{
				newPosition.y = sc->Size.y / 2.0f;
			}
			else if (newPosition.y + sc->Size.y / 2.0f > HurricaneConstants::Height)
			{
				newPosition.y = HurricaneConstants::Height - sc->Size.y / 2.0f;
			}

			tc->SetPosition({ newPosition, pos.z });
		}

		if (boostActive)
		{
			if (_scene.getEntity(PlayerEngineEntityName) == nullptr)
			{
				spawnPlayerEngine();
			}
			syncPlayerEngine(*player);
		}
		else
		{
			destroyPlayerEngine();
		}

		const bool shieldActive = loadout != nullptr && loadout->shieldLayers > 0;
		if (shieldActive)
		{
			if (_scene.getEntity(PlayerShieldEntityName) == nullptr)
			{
				spawnPlayerShield();
			}
			syncPlayerShield(*player, loadout->shieldLayers);
		}
		else
		{
			destroyPlayerShield();
		}
	}

}
