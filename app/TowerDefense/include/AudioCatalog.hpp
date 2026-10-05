#pragma once

#include <helsinki/Audio/Audio.hpp>
#include <string>

namespace tower
{
	inline constexpr const char* CuePlace = "place";
	inline constexpr const char* CueLeak = "leak";
	inline constexpr const char* CueWin = "win";

	inline bool loadTowerCue(
		hl::audio::Audio& audio,
		const char* id,
		const std::string& root,
		const char* stem)
	{
		const std::string ogg = root + "/data/audio/" + stem + ".ogg";
		const std::string wav = root + "/data/audio/" + stem + ".wav";
		return audio.loadClip(id, ogg) || audio.loadClip(id, wav);
	}

	inline void loadTowerAudio(hl::audio::Audio& audio, const std::string& root)
	{
		loadTowerCue(audio, CuePlace, root, "place");
		loadTowerCue(audio, CueLeak, root, "leak");
		loadTowerCue(audio, CueWin, root, "win");
	}
}
