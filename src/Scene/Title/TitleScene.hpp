#pragma once

#include <Scene/Scene.hpp>

class TitleScene : public Scene
{
public:
	TitleScene();
	~TitleScene();

	void update(float delta) override;
	void draw() override;
};