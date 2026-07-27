#pragma once

namespace hur
{
	class HurricaneConstants
	{
		HurricaneConstants() = delete;
	public:
		static const constexpr int Width = 800;
		static const constexpr int Height = 600;
	};

	enum class GameState
	{
		INIT = 0,
		PLAYING = 1,
		GAME_OVER = 2
	};
}