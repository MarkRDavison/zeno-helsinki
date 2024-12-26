#include <Scene/Title/TitleScene.hpp>
#include <string>
#include <raylib.h>
#include <Scene/SceneManager.hpp>
#include <Scene/Game/GameScene.hpp>

TitleScene::TitleScene()
{

}

TitleScene::~TitleScene()
{

}

void TitleScene::update(float delta)
{
	if (IsKeyPressed(KEY_C))
	{
		SceneManager::setCurrentScene(std::make_unique<GameScene>());
	}
}

void TitleScene::draw()
{
	auto draw_with_backdrop = [](const std::string& text, int x, int y, int fontSize, Color color, Color backdropColor)
		{
			DrawText(text.c_str(), x + 1, y + 1, fontSize, backdropColor);
			DrawText(text.c_str(), x, y, fontSize, color);
		};

	draw_with_backdrop("Press 'c' to play!", 10, 10, 30, GOLD, BLACK);

	

	const std::string text = "This is the Title Scene";
	const int text_width = MeasureText(text.c_str(), 20);
	DrawText(text.c_str(), GetScreenWidth() / 2 - text_width / 2, GetScreenHeight() / 2, 20, BLACK);
}
