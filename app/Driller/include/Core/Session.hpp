#pragma once

#include <string>

namespace drl
{

	class Session
	{
	public:
		void loadGameSettings(const std::string& gameJsonPath);
		void loadAndValidate();

		bool skipToGameplay() const { return _skipToGameplay; }
		float simSpeed() const { return _simSpeed; }
		bool loadSucceeded() const { return _loadSucceeded; }

	private:
		bool _settingsLoaded{ false };
		bool _loadSucceeded{ false };
		bool _skipToGameplay{ false };
		float _simSpeed{ 1.0f };
	};

}
