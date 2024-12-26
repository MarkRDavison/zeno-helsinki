#pragma once

class Scene
{
public:
    virtual ~Scene() = default;
    virtual void update(float delta) = 0;
    virtual void draw() = 0;
};