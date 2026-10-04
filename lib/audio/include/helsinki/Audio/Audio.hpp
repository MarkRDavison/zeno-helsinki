#pragma once

#include <helsinki/System/Utils/NonCopyable.hpp>
#include <memory>
#include <string_view>

namespace hl::audio
{
	struct AudioInit
	{
		bool openDevice = true;
	};

	class Audio : public hl::NonCopyable
	{
	public:
		Audio();
		~Audio() override;

		bool init();
		bool init(AudioInit options);
		void shutdown();
		bool ok() const;

		bool loadClip(std::string_view id, std::string_view path);
		bool loadStream(std::string_view id, std::string_view path);

		void play(std::string_view id);
		void playLoop(std::string_view id);
		void stopMusic();
		void stopSfx();

		void setMasterVolume(float volume);
		float masterVolume() const;
		void setSfxVolume(float volume);
		float sfxVolume() const;
		void setMusicVolume(float volume);
		float musicVolume() const;
		void setMusicMuted(bool muted);
		bool musicMuted() const;
		void setPaused(bool paused);
		bool paused() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
	};
}
