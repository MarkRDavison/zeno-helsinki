#pragma once

#include <helsinki/Audio/Audio.hpp>
#include <string>

namespace hur
{
	inline constexpr const char* CuePlayerShoot = "player_shoot";
	inline constexpr const char* CueEnemyShoot = "enemy_shoot";
	inline constexpr const char* CueEnemyHeavy = "enemy_heavy";
	inline constexpr const char* CueBombLaunch = "bomb_launch";
	inline constexpr const char* CueExplode = "explode";
	inline constexpr const char* CueEnemyDeath = "enemy_death";
	inline constexpr const char* CuePlayerLifeLost = "player_life_lost";
	inline constexpr const char* CuePickup = "pickup";
	inline constexpr const char* CueMusicTitle = "music_title";
	inline constexpr const char* CueMusicGame = "music_game";

	inline bool loadHurricaneCue(
		hl::audio::Audio& audio,
		const char* id,
		const std::string& root,
		const char* stem,
		bool stream)
	{
		const std::string ogg = root + "/data/audio/" + stem + ".ogg";
		const std::string wav = root + "/data/audio/" + stem + ".wav";
		if (stream)
		{
			return audio.loadStream(id, ogg) || audio.loadStream(id, wav);
		}
		return audio.loadClip(id, ogg) || audio.loadClip(id, wav);
	}

	inline void loadHurricaneAudio(hl::audio::Audio& audio, const std::string& root)
	{
		loadHurricaneCue(audio, CuePlayerShoot, root, "player_shoot", false);
		loadHurricaneCue(audio, CueEnemyShoot, root, "enemy_shoot", false);
		loadHurricaneCue(audio, CueEnemyHeavy, root, "enemy_heavy", false);
		loadHurricaneCue(audio, CueBombLaunch, root, "bomb_launch", false);
		loadHurricaneCue(audio, CueExplode, root, "explode", false);
		loadHurricaneCue(audio, CueEnemyDeath, root, "enemy_death", false);
		loadHurricaneCue(audio, CuePlayerLifeLost, root, "player_life_lost", false);
		loadHurricaneCue(audio, CuePickup, root, "pickup", false);
		loadHurricaneCue(audio, CueMusicTitle, root, "music_title", true);
		loadHurricaneCue(audio, CueMusicGame, root, "music_game", true);
	}
}
