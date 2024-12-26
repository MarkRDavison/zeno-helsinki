#include <cstdlib>
#include <iostream>
#include <raylib.h>
#include <Constants.hpp>
#include <Scene/SceneManager.hpp>
#include <Scene/Title/TitleScene.hpp>


int main(int argc, char** argv)
{
    int width{ 1280 }, height{ 720 };
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, Constants::Title.c_str());

    SetTargetFPS(60); 

    SceneManager::setCurrentScene(std::make_unique<TitleScene>());

    while (!WindowShouldClose())
    {
        const auto delta = GetFrameTime();

        SceneManager::update(delta);

        BeginDrawing();
        ClearBackground(SKYBLUE);

        SceneManager::draw();

        EndDrawing();
    };
}