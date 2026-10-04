#pragma once

#include <helsinki/Audio/Audio.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/System/Events/EventBus.hpp>
#include <helsinki/System/Events/EventListener.hpp>

namespace hur
{
	class AudioListener : public hl::EventListener
	{
	public:
		AudioListener(
			hl::EventBus& eventBus,
			hl::Scene& scene,
			hl::audio::Audio& audio);
		~AudioListener();

		void OnEvent(const hl::Event& event) override;

	private:
		hl::EventBus& _eventBus;
		hl::Scene& _scene;
		hl::audio::Audio& _audio;
	};
}
