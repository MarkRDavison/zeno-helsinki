#pragma once

namespace pong
{

	class PongConstants
	{
		PongConstants() = delete;
	public:
		static const int Offset = 64;

		static const int PaddleWidth = 24;
		static const int PaddleHeight = 96;

		static const int BallSize = 32;

		static const int GameBoundsWidth = 800;
		static const int GameBoundsHeight = 600;

		static const int GameBoundsWallWidth = 24;
	};


	enum class GameState
	{
		INIT = 0,
		PLAYING = 1,
		GAME_OVER = 2
	};
}