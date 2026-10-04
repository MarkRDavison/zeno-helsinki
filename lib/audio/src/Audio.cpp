#include <helsinki/Audio/Audio.hpp>

#include <miniaudio.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>

namespace hl::audio
{
	namespace
	{
		constexpr std::size_t kSfxVoiceCap = 32;

		float clamp01(float value)
		{
			return std::clamp(value, 0.0f, 1.0f);
		}

		bool decodeFileOk(const char* path)
		{
			ma_decoder decoder;
			const ma_result result = ma_decoder_init_file(path, nullptr, &decoder);
			if (result != MA_SUCCESS)
			{
				return false;
			}
			ma_decoder_uninit(&decoder);
			return true;
		}
	}

	struct Audio::Impl
	{
		struct Clip
		{
			std::string path;
			bool stream = false;
		};

		struct Voice
		{
			ma_sound sound{};
			bool inUse = false;
			std::uint64_t order = 0;
		};

		ma_engine engine{};
		ma_sound_group sfxGroup{};
		ma_sound_group musicGroup{};
		ma_sound music{};
		bool engineReady = false;
		bool groupsReady = false;
		bool musicReady = false;
		bool paused = false;
		bool musicMuted = false;
		float masterVolume = 0.7f;
		float sfxVolume = 1.0f;
		float musicVolume = 1.0f;
		std::uint64_t nextOrder = 1;
		std::unordered_map<std::string, Clip> clips;
		std::array<Voice, kSfxVoiceCap> voices{};

		float musicBusVolume() const
		{
			return musicMuted ? 0.0f : musicVolume;
		}

		void applyVolumes()
		{
			if (!engineReady)
			{
				return;
			}
			ma_engine_set_volume(&engine, masterVolume);
			if (groupsReady)
			{
				ma_sound_group_set_volume(&sfxGroup, sfxVolume);
				ma_sound_group_set_volume(&musicGroup, musicBusVolume());
			}
		}

		void applyPaused()
		{
			if (!engineReady)
			{
				return;
			}
			for (auto& voice : voices)
			{
				if (voice.inUse)
				{
					if (paused)
					{
						ma_sound_stop(&voice.sound);
					}
					else
					{
						ma_sound_start(&voice.sound);
					}
				}
			}
			if (musicReady)
			{
				if (paused)
				{
					ma_sound_stop(&music);
				}
				else
				{
					ma_sound_start(&music);
				}
			}
		}

		void uninitVoice(Voice& voice)
		{
			if (voice.inUse)
			{
				ma_sound_uninit(&voice.sound);
				voice.inUse = false;
				voice.order = 0;
			}
		}

		void uninitMusic()
		{
			if (musicReady)
			{
				ma_sound_uninit(&music);
				musicReady = false;
			}
		}

		Voice* acquireVoice()
		{
			Voice* free = nullptr;
			Voice* oldest = nullptr;
			for (auto& voice : voices)
			{
				if (!voice.inUse)
				{
					free = &voice;
					break;
				}
				if (oldest == nullptr || voice.order < oldest->order)
				{
					oldest = &voice;
				}
			}
			if (free != nullptr)
			{
				return free;
			}
			if (oldest != nullptr)
			{
				uninitVoice(*oldest);
				return oldest;
			}
			return nullptr;
		}
	};

	Audio::Audio() : _impl(std::make_unique<Impl>())
	{
	}

	Audio::~Audio()
	{
		shutdown();
	}

	bool Audio::init()
	{
		return init(AudioInit{});
	}

	bool Audio::init(AudioInit options)
	{
		shutdown();

		ma_engine_config config = ma_engine_config_init();
		if (!options.openDevice)
		{
			config.noDevice = MA_TRUE;
			config.channels = 2;
			config.sampleRate = 48000;
		}

		const ma_result result = ma_engine_init(&config, &_impl->engine);
		if (result != MA_SUCCESS)
		{
			std::cerr << "hl::audio: ma_engine_init failed (" << static_cast<int>(result) << ")\n";
			return false;
		}
		_impl->engineReady = true;

		const ma_result sfxGroupResult = ma_sound_group_init(&_impl->engine, 0, nullptr, &_impl->sfxGroup);
		const ma_result musicGroupResult = ma_sound_group_init(&_impl->engine, 0, nullptr, &_impl->musicGroup);
		if (sfxGroupResult != MA_SUCCESS || musicGroupResult != MA_SUCCESS)
		{
			std::cerr << "hl::audio: ma_sound_group_init failed\n";
			shutdown();
			return false;
		}
		_impl->groupsReady = true;
		_impl->applyVolumes();
		return true;
	}

	void Audio::shutdown()
	{
		if (_impl == nullptr)
		{
			return;
		}

		stopSfx();
		_impl->uninitMusic();

		if (_impl->groupsReady)
		{
			ma_sound_group_uninit(&_impl->sfxGroup);
			ma_sound_group_uninit(&_impl->musicGroup);
			_impl->groupsReady = false;
		}

		if (_impl->engineReady)
		{
			ma_engine_uninit(&_impl->engine);
			_impl->engineReady = false;
		}

		_impl->clips.clear();
		_impl->paused = false;
	}

	bool Audio::ok() const
	{
		return _impl->engineReady;
	}

	bool Audio::loadClip(std::string_view id, std::string_view path)
	{
		if (!ok())
		{
			return false;
		}

		const std::string pathStr(path);
		if (!decodeFileOk(pathStr.c_str()))
		{
			std::cerr << "hl::audio: failed to load clip '" << std::string(id) << "' from " << pathStr << "\n";
			return false;
		}

		_impl->clips.insert_or_assign(std::string(id), Impl::Clip{ pathStr, false });
		return true;
	}

	bool Audio::loadStream(std::string_view id, std::string_view path)
	{
		if (!ok())
		{
			return false;
		}

		const std::string pathStr(path);
		if (!decodeFileOk(pathStr.c_str()))
		{
			std::cerr << "hl::audio: failed to load stream '" << std::string(id) << "' from " << pathStr << "\n";
			return false;
		}

		_impl->clips.insert_or_assign(std::string(id), Impl::Clip{ pathStr, true });
		return true;
	}

	void Audio::play(std::string_view id)
	{
		if (!ok())
		{
			return;
		}

		const auto it = _impl->clips.find(std::string(id));
		if (it == _impl->clips.end())
		{
			return;
		}

		Impl::Voice* voice = _impl->acquireVoice();
		if (voice == nullptr)
		{
			return;
		}

		ma_uint32 flags = MA_SOUND_FLAG_NO_SPATIALIZATION;
		if (!it->second.stream)
		{
			flags |= MA_SOUND_FLAG_DECODE;
		}
		else
		{
			flags |= MA_SOUND_FLAG_STREAM;
		}

		const ma_result result = ma_sound_init_from_file(
			&_impl->engine,
			it->second.path.c_str(),
			flags,
			&_impl->sfxGroup,
			nullptr,
			&voice->sound);
		if (result != MA_SUCCESS)
		{
			return;
		}

		voice->inUse = true;
		voice->order = _impl->nextOrder++;
		if (!_impl->paused)
		{
			ma_sound_start(&voice->sound);
		}
	}

	void Audio::playLoop(std::string_view id)
	{
		if (!ok())
		{
			return;
		}

		_impl->uninitMusic();

		const auto it = _impl->clips.find(std::string(id));
		if (it == _impl->clips.end())
		{
			return;
		}

		ma_uint32 flags = MA_SOUND_FLAG_NO_SPATIALIZATION;
		flags |= it->second.stream ? MA_SOUND_FLAG_STREAM : MA_SOUND_FLAG_DECODE;

		const ma_result result = ma_sound_init_from_file(
			&_impl->engine,
			it->second.path.c_str(),
			flags,
			&_impl->musicGroup,
			nullptr,
			&_impl->music);
		if (result != MA_SUCCESS)
		{
			return;
		}

		_impl->musicReady = true;
		ma_sound_set_looping(&_impl->music, MA_TRUE);
		if (!_impl->paused)
		{
			ma_sound_start(&_impl->music);
		}
	}

	void Audio::stopMusic()
	{
		_impl->uninitMusic();
	}

	void Audio::stopSfx()
	{
		for (auto& voice : _impl->voices)
		{
			_impl->uninitVoice(voice);
		}
	}

	void Audio::setMasterVolume(float volume)
	{
		_impl->masterVolume = clamp01(volume);
		_impl->applyVolumes();
	}

	float Audio::masterVolume() const
	{
		return _impl->masterVolume;
	}

	void Audio::setSfxVolume(float volume)
	{
		_impl->sfxVolume = clamp01(volume);
		_impl->applyVolumes();
	}

	float Audio::sfxVolume() const
	{
		return _impl->sfxVolume;
	}

	void Audio::setMusicVolume(float volume)
	{
		_impl->musicVolume = clamp01(volume);
		_impl->applyVolumes();
	}

	float Audio::musicVolume() const
	{
		return _impl->musicVolume;
	}

	void Audio::setMusicMuted(bool muted)
	{
		_impl->musicMuted = muted;
		_impl->applyVolumes();
	}

	bool Audio::musicMuted() const
	{
		return _impl->musicMuted;
	}

	void Audio::setPaused(bool paused)
	{
		_impl->paused = paused;
		_impl->applyPaused();
	}

	bool Audio::paused() const
	{
		return _impl->paused;
	}
}
