#pragma once

#include <Scene/Scene.hpp>

class GameScene : public Scene
{
public:
	GameScene();
	~GameScene();

	void update(float delta) override;
	void draw() override;
};