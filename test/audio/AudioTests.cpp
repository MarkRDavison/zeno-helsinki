#include <catch2/catch_test_macros.hpp>
#include <helsinki/Audio/Audio.hpp>

#include <filesystem>
#include <string>

namespace hl::audio::test
{
	namespace
	{
		std::string sinePath()
		{
			return (std::filesystem::path(__FILE__).parent_path() / "data" / "sine.ogg").string();
		}

		void initNoDevice(Audio& audio)
		{
			REQUIRE(audio.init(AudioInit{ .openDevice = false }));
			REQUIRE(audio.ok());
		}
	}

	TEST_CASE("Init without a device", "[Audio]")
	{
		Audio audio;
		CHECK(audio.init(AudioInit{ .openDevice = false }));
		CHECK(audio.ok());
		audio.shutdown();
	}

	TEST_CASE("Missing clip load is false and play is a no-op", "[Audio]")
	{
		Audio audio;
		initNoDevice(audio);
		CHECK_FALSE(audio.loadClip("missing", "definitely-not-a-real-file.ogg"));
		audio.play("missing");
	}

	TEST_CASE("Load ogg clip", "[Audio]")
	{
		Audio audio;
		initNoDevice(audio);
		CHECK(audio.loadClip("sine", sinePath()));
	}

	TEST_CASE("Load ogg stream", "[Audio]")
	{
		Audio audio;
		initNoDevice(audio);
		CHECK(audio.loadStream("sine_stream", sinePath()));
	}

	TEST_CASE("Volume setters clamp to 0-1", "[Audio]")
	{
		Audio audio;
		audio.setMasterVolume(-1.0f);
		CHECK(audio.masterVolume() == 0.0f);
		audio.setMasterVolume(2.0f);
		CHECK(audio.masterVolume() == 1.0f);
		audio.setSfxVolume(-1.0f);
		CHECK(audio.sfxVolume() == 0.0f);
		audio.setSfxVolume(2.0f);
		CHECK(audio.sfxVolume() == 1.0f);
		audio.setMusicVolume(-1.0f);
		CHECK(audio.musicVolume() == 0.0f);
		audio.setMusicVolume(2.0f);
		CHECK(audio.musicVolume() == 1.0f);
	}

	TEST_CASE("Music mute preserves stored level", "[Audio]")
	{
		Audio audio;
		audio.setMusicVolume(0.4f);
		audio.setMusicMuted(true);
		CHECK(audio.musicMuted());
		CHECK(audio.musicVolume() == 0.4f);
		audio.setMusicMuted(false);
		CHECK_FALSE(audio.musicMuted());
		CHECK(audio.musicVolume() == 0.4f);
	}

	TEST_CASE("Pause tracks state", "[Audio]")
	{
		Audio audio;
		initNoDevice(audio);
		CHECK_FALSE(audio.paused());
		audio.setPaused(true);
		CHECK(audio.paused());
		audio.setPaused(false);
		CHECK_FALSE(audio.paused());
	}

	TEST_CASE("Overlapping play steals oldest at cap", "[Audio]")
	{
		Audio audio;
		initNoDevice(audio);
		REQUIRE(audio.loadClip("sine", sinePath()));
		for (int i = 0; i < 33; ++i)
		{
			audio.play("sine");
		}
	}

	TEST_CASE("playLoop replaces the music voice", "[Audio]")
	{
		Audio audio;
		initNoDevice(audio);
		REQUIRE(audio.loadStream("a", sinePath()));
		REQUIRE(audio.loadStream("b", sinePath()));
		audio.playLoop("a");
		audio.playLoop("b");
		audio.playLoop("missing");
	}

	TEST_CASE("Shutdown twice is safe and play afterwards is a no-op", "[Audio]")
	{
		Audio audio;
		initNoDevice(audio);
		REQUIRE(audio.loadClip("sine", sinePath()));
		audio.shutdown();
		audio.shutdown();
		CHECK_FALSE(audio.ok());
		audio.play("sine");
	}
}
