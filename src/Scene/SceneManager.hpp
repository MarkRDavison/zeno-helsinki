#pragma once
#include <memory>
#include <Scene/Scene.hpp>

class SceneManager
{
private:
	static std::unique_ptr<Scene> _currentScene;
public:
	static void initialize();
	static void setCurrentScene(std::unique_ptr<Scene> scene);

	static void update(float delta);
	static void draw();
	static void cleanup();

};