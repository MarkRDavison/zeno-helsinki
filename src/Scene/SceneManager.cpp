#include <Scene/SceneManager.hpp>

std::unique_ptr<Scene> SceneManager::_currentScene = nullptr;

void SceneManager::initialize()
{
	_currentScene = nullptr;
}

void SceneManager::setCurrentScene(std::unique_ptr<Scene> scene)
{
	_currentScene.reset();
	_currentScene = std::move(scene);
}

void SceneManager::update(float delta)
{
	if (_currentScene != nullptr)
	{
		_currentScene->update(delta);
	}
}
void SceneManager::draw()
{
	if (_currentScene != nullptr)
	{
		_currentScene->draw();
	}
}
void SceneManager::cleanup()
{
	_currentScene = nullptr;
}