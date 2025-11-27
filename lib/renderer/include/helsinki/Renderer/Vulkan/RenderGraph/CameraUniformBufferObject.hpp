#pragma once

#include <helsinki/System/glm.hpp>

namespace hl
{

    struct CameraUniformBufferObject
    {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

}