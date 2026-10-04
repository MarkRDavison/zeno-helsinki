#include <AudioListener.hpp>
#include <AudioCatalog.hpp>
#include <Events/BombEvent.hpp>
#include <Events/EntityDeathEvent.hpp>
#include <Events/MissileExplodeEvent.hpp>
#include <Events/PlayerLifeLostEvent.hpp>
#include <Events/ShootEvent.hpp>
#include <WeaponCatalog.hpp>

namespace hur
{
	AudioListener::AudioListener(
		hl::EventBus& eventBus,
		hl::Scene& scene,
		hl::audio::Audio& audio
	) :
		_eventBus(eventBus),
		_scene(scene),
		_audio(audio)
	{
		_eventBus.AddListener(this);
	}

	AudioListener::~AudioListener()
	{
		_eventBus.RemoveListener(this);
	}

	void AudioListener::OnEvent(const hl::Event& event)
	{
		if (auto se = dynamic_cast<const ShootEvent*>(&event))
		{
			auto* shooter = _scene.getEntity(se->getShooterId());
			if (shooter == nullptr)
			{
				return;
			}

			if (shooter->HasTag("PLAYER"))
			{
				_audio.play(CuePlayerShoot);
				return;
			}

			auto* weapon = shooter->GetComponent<WeaponComponent>();
			if (weapon != nullptr && weapon->Type == WeaponTypeEnemyHeavyMissile)
			{
				_audio.play(CueEnemyHeavy);
				return;
			}

			_audio.play(CueEnemyShoot);
			return;
		}

		if (dynamic_cast<const BombEvent*>(&event) != nullptr)
		{
			_audio.play(CueBombLaunch);
			return;
		}

		if (dynamic_cast<const MissileExplodeEvent*>(&event) != nullptr)
		{
			_audio.play(CueExplode);
			return;
		}

		if (auto ede = dynamic_cast<const EntityDeathEvent*>(&event))
		{
			auto* entity = _scene.getEntity(ede->getId());
			if (entity == nullptr || entity->HasTag("PLAYER"))
			{
				return;
			}
			_audio.play(CueEnemyDeath);
			return;
		}

		if (dynamic_cast<const PlayerLifeLostEvent*>(&event) != nullptr)
		{
			_audio.play(CuePlayerLifeLost);
		}
	}
}
